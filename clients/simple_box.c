#define _GNU_SOURCE 1
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>
#include <wayland-client.h>

#include "z11-client-protocol.h"

struct z11_compositor *compositor = NULL;
struct wl_shm *shm = NULL;
struct z11_gl *gl = NULL;

void shm_format(void *data, struct wl_shm *wl_shm, uint32_t format) {}

struct wl_shm_listener shm_listener = {
    shm_format,
};

int create_shared_fd(off_t size)
{
  char name[1024] = "";

  int fd = memfd_create("z11-simple-box", MFD_CLOEXEC | MFD_ALLOW_SEALING);
  if (fd < 0) {
    fprintf(stderr, "File cannot open: %s\n", name);
    exit(1);
  } else {
    unlink(name);
  }

  if (ftruncate(fd, size) < 0) {
    fprintf(stderr, "ftruncate failed: fd=%i, size=%li\n", fd, size);
    close(fd);
    exit(1);
  }

  return fd;
}

void global_registry_handler(void *data, struct wl_registry *registry, uint32_t id, const char *interface,
                             uint32_t version)
{
  if (strcmp(interface, "z11_compositor") == 0) {
    compositor = wl_registry_bind(registry, id, &z11_compositor_interface, version);
  } else if (strcmp(interface, "wl_shm") == 0) {
    shm = wl_registry_bind(registry, id, &wl_shm_interface, 1);
    wl_shm_add_listener(shm, &shm_listener, NULL);
  } else if (strcmp(interface, "z11_gl") == 0) {
    gl = wl_registry_bind(registry, id, &z11_gl_interface, 1);
  }
}

void global_registry_remover(void *data, struct wl_registry *registry, uint32_t id) {}

const struct wl_registry_listener registry_listener = {
    global_registry_handler,
    global_registry_remover,
};

typedef struct {
  float x, y, z;
} Point;

typedef struct {
  Point start, end;
} Line;

typedef struct {
  Point p1, p2, p3;
} Triangle;

static void paint_plane(Triangle *out, float x, float y, float z, float theta)
{
  double root2x5 = sqrt(2) * 5;
  double root2x5_cos = root2x5 * cos(theta + M_PI / 4);
  double root2x5_sin = root2x5 * sin(theta + M_PI / 4);

  Point A = {x - root2x5_cos, y - 5, z - root2x5_sin};
  Point B = {x + root2x5_sin, y - 5, z - root2x5_cos};
  Point C = {x + root2x5_sin, y + 5, z - root2x5_cos};
  Point D = {x - root2x5_cos, y + 5, z - root2x5_sin};

  out->p1 = A;
  out->p2 = B;
  out->p3 = C;
  out++;
  out->p1 = A;
  out->p2 = D;
  out->p3 = C;
}

static void paint_frame(Line *out, float x, float y, float z, float theta)
{
  double root2x5 = sqrt(2) * 5;
  double root2x5_cos = root2x5 * cos(theta + M_PI / 4);
  double root2x5_sin = root2x5 * sin(theta + M_PI / 4);

  Point A = {x - root2x5_cos, y - 5, z - root2x5_sin};
  Point B = {x + root2x5_sin, y - 5, z - root2x5_cos};
  Point C = {x + root2x5_sin, y + 5, z - root2x5_cos};
  Point D = {x - root2x5_cos, y + 5, z - root2x5_sin};
  Point E = {x - root2x5_sin, y - 5, z + root2x5_cos};
  Point F = {x + root2x5_cos, y - 5, z + root2x5_sin};
  Point G = {x + root2x5_cos, y + 5, z + root2x5_sin};
  Point H = {x - root2x5_sin, y + 5, z + root2x5_cos};

  out->start = A;
  out->end = B;
  out++;
  out->start = B;
  out->end = C;
  out++;
  out->start = C;
  out->end = D;
  out++;
  out->start = D;
  out->end = A;
  out++;

  out->start = E;
  out->end = F;
  out++;
  out->start = F;
  out->end = G;
  out++;
  out->start = G;
  out->end = H;
  out++;
  out->start = H;
  out->end = E;
  out++;

  out->start = A;
  out->end = E;
  out++;
  out->start = B;
  out->end = F;
  out++;
  out->start = C;
  out->end = G;
  out++;
  out->start = D;
  out->end = H;
}

const char *vertex_shader;

const char *red_fragment_shader;

const char *orange_fragment_shader;

int main(int argc, char const *argv[])
{
  int x = 0;
  int y = 0;
  int z = 20;
  double del_theta = 0;
  double theta = 0;
  if (argc > 1) x = atoi(argv[1]);
  if (argc > 2) y = atoi(argv[2]);
  if (argc > 3) z = atoi(argv[3]);
  if (argc > 4) del_theta = atof(argv[4]) / 100;

  struct wl_display *display = wl_display_connect(NULL);
  if (display == NULL) {
    fprintf(stderr, "Can't connect to display\n");
    exit(1);
  }
  fprintf(stdout, "connected to display\n");

  struct wl_registry *registry = wl_display_get_registry(display);
  wl_registry_add_listener(registry, &registry_listener, NULL);

  wl_display_dispatch(display);
  wl_display_roundtrip(display);

  assert(compositor && shm);

  int size_of_lines = sizeof(Line) * 12;
  int size_of_triangles = sizeof(Triangle) * 2;
  int fd = create_shared_fd(size_of_lines + size_of_triangles);
  void *shm_data = mmap(NULL, size_of_lines + size_of_triangles, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (shm_data == MAP_FAILED) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
    fprintf(stderr, "line mmap failed: %m\n");
#pragma GCC diagnostic pop
    close(fd);
    exit(1);
  }
  Line *shm_line_data = (Line *)shm_data;
  paint_frame(shm_line_data, x, y, z, theta);

  Triangle *shm_triangle_data = (Triangle *)((char *)shm_data + size_of_lines);
  paint_plane(shm_triangle_data, x, y, z, theta);

  struct wl_shm_pool *pool = wl_shm_create_pool(shm, fd, size_of_lines + size_of_triangles);
  struct wl_raw_buffer *line_buffer = wl_shm_pool_create_raw_buffer(pool, 0, size_of_lines);
  struct wl_raw_buffer *triangle_buffer =
      wl_shm_pool_create_raw_buffer(pool, size_of_lines, size_of_triangles);
  wl_shm_pool_destroy(pool);

  struct z11_gl_vertex_buffer *line_vertex_buffer = z11_gl_create_vertex_buffer(gl);
  z11_gl_vertex_buffer_allocate(line_vertex_buffer, size_of_lines, line_buffer);

  struct z11_gl_vertex_buffer *triangle_vertex_buffer = z11_gl_create_vertex_buffer(gl);
  z11_gl_vertex_buffer_allocate(triangle_vertex_buffer, size_of_triangles, triangle_buffer);

  struct z11_gl_shader_program *frame_shader_program =
      z11_gl_create_shader_program(gl, vertex_shader, red_fragment_shader);

  struct z11_gl_shader_program *plane_shader_program =
      z11_gl_create_shader_program(gl, vertex_shader, orange_fragment_shader);

  struct z11_render_block *frame_render_block = z11_compositor_create_render_block(compositor);
  z11_render_block_attach_vertex_buffer(frame_render_block, line_vertex_buffer);
  z11_render_block_attach_shader_program(frame_render_block, frame_shader_program);
  z11_render_block_commit(frame_render_block);

  struct z11_render_block *plane_render_block = z11_compositor_create_render_block(compositor);
  z11_render_block_attach_vertex_buffer(plane_render_block, triangle_vertex_buffer);
  z11_render_block_attach_shader_program(plane_render_block, plane_shader_program);
  z11_render_block_set_topology(plane_render_block, Z11_GL_TOPOLOGY_TRIANGLES);
  z11_render_block_commit(plane_render_block);

  struct timeval base, now;
  gettimeofday(&base, NULL);

  int ret;
  while (wl_display_dispatch_pending(display) != -1) {
    ret = wl_display_flush(display);
    if (ret == -1) break;

    gettimeofday(&now, NULL);
    if ((now.tv_sec - base.tv_sec) * 1000000 + now.tv_usec - base.tv_usec > 16666) {  // 60 hz
      theta += del_theta;
      if (theta >= 2 * M_PI || theta <= -2 * M_PI) theta = 0;
      base = now;
      paint_frame(shm_line_data, x, y, z, theta);
      z11_gl_vertex_buffer_allocate(line_vertex_buffer, size_of_lines, line_buffer);
      paint_plane(shm_triangle_data, x, y, z, theta);
      z11_gl_vertex_buffer_allocate(triangle_vertex_buffer, size_of_triangles, triangle_buffer);
    }
  }

  return 0;
}

const char *vertex_shader =
    "#version 410\n"
    "uniform mat4 matrix;\n"
    "layout(location = 0) in vec4 position;\n"
    "layout(location = 1) in vec2 v2UVcoordsIn;\n"
    "layout(location = 2) in vec3 v3NormalIn;\n"
    "out vec2 v2UVcoords;\n"
    "void main()\n"
    "{\n"
    "  v2UVcoords = v2UVcoordsIn;\n"
    "  gl_Position = matrix * position;\n"
    "}\n";

const char *red_fragment_shader =
    "#version 410 core\n"
    "in vec2 v2UVcoords;\n"
    "out vec4 outputColor;\n"
    "void main()\n"
    "{\n"
    "  outputColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "}\n";

const char *orange_fragment_shader =
    "#version 410 core\n"
    "in vec2 v2UVcoords;\n"
    "out vec4 outputColor;\n"
    "void main()\n"
    "{\n"
    "  outputColor = vec4(1.0, 0.647, 0.0, 1.0);\n"
    "}\n";
