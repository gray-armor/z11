#define _GNU_SOURCE 1
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>
#include <wayland-client.h>

#include "helper.h"
#include "z11-client-protocol.h"
#include "z11-opengl-client-protocol.h"

typedef struct {
  Point p1, p2, p3;
} Triangle;

static void paint_plane(Triangle *out, float x, float y, float z, float theta);
static void paint_frame(Line *out, float x, float y, float z, float theta);
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

  struct z11_global *global = z_helper_global();

  // prepare shared memory
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
  Triangle *shm_triangle_data = (Triangle *)((char *)shm_data + size_of_lines);

  // prepare buffer objects
  struct wl_shm_pool *pool = wl_shm_create_pool(global->shm, fd, size_of_lines + size_of_triangles);
  struct wl_raw_buffer *line_buffer = wl_shm_pool_create_raw_buffer(pool, 0, size_of_lines);
  struct wl_raw_buffer *triangle_buffer =
      wl_shm_pool_create_raw_buffer(pool, size_of_lines, size_of_triangles);
  wl_shm_pool_destroy(pool);

  // prepare vertex buffers
  struct z11_opengl_vertex_buffer *line_vertex_buffer = z11_opengl_create_vertex_buffer(global->gl);
  struct z11_opengl_vertex_buffer *triangle_vertex_buffer = z11_opengl_create_vertex_buffer(global->gl);

  // prepare shaders
  struct z11_opengl_shader_program *frame_shader_program =
      z11_opengl_create_shader_program(global->gl, vertex_shader, red_fragment_shader);

  struct z11_opengl_shader_program *plane_shader_program =
      z11_opengl_create_shader_program(global->gl, vertex_shader, orange_fragment_shader);

  // prepare virtual object
  struct z11_virtual_object *virtual_object = z11_compositor_create_virtual_object(global->compositor);

  // prepare render component
  struct z11_opengl_render_component *frame_render_component =
      z11_opengl_render_component_manager_create_opengl_render_component(global->render_component_manager,
                                                                         virtual_object);
  z11_opengl_render_component_attach_vertex_buffer(frame_render_component, line_vertex_buffer);
  z11_opengl_render_component_attach_shader_program(frame_render_component, frame_shader_program);
  z11_opengl_render_component_append_vertex_input_attribute(
      frame_render_component, 0, Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR3, 0);

  struct z11_opengl_render_component *plane_render_component =
      z11_opengl_render_component_manager_create_opengl_render_component(global->render_component_manager,
                                                                         virtual_object);
  z11_opengl_render_component_attach_vertex_buffer(plane_render_component, triangle_vertex_buffer);
  z11_opengl_render_component_attach_shader_program(plane_render_component, plane_shader_program);
  z11_opengl_render_component_append_vertex_input_attribute(
      plane_render_component, 0, Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR3, 0);
  z11_opengl_render_component_set_topology(plane_render_component, Z11_OPENGL_TOPOLOGY_TRIANGLES);

  // first render
  paint_frame(shm_line_data, x, y, z, theta);
  paint_plane(shm_triangle_data, x, y, z, theta);
  z11_opengl_vertex_buffer_attach(line_vertex_buffer, line_buffer, sizeof(Point));
  z11_opengl_vertex_buffer_attach(triangle_vertex_buffer, triangle_buffer, sizeof(Point));
  z11_virtual_object_commit(virtual_object);

  struct timeval base, now;
  gettimeofday(&base, NULL);

  int ret;
  while (wl_display_dispatch_pending(global->display) != -1) {
    ret = wl_display_flush(global->display);
    if (ret == -1) break;

    gettimeofday(&now, NULL);
    if ((now.tv_sec - base.tv_sec) * 1000000 + now.tv_usec - base.tv_usec > 16666) {  // 60 hz
      // TODO: Enable to sync with compositor cycle
      theta += del_theta;
      if (theta >= 2 * M_PI || theta <= -2 * M_PI) theta = 0;
      base = now;
      paint_frame(shm_line_data, x, y, z, theta);
      paint_plane(shm_triangle_data, x, y, z, theta);
      z11_opengl_vertex_buffer_attach(line_vertex_buffer, line_buffer, sizeof(Point));
      z11_opengl_vertex_buffer_attach(triangle_vertex_buffer, triangle_buffer, sizeof(Point));
      z11_virtual_object_commit(virtual_object);
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

  out->s = A;
  out->e = B;
  out++;
  out->s = B;
  out->e = C;
  out++;
  out->s = C;
  out->e = D;
  out++;
  out->s = D;
  out->e = A;
  out++;

  out->s = E;
  out->e = F;
  out++;
  out->s = F;
  out->e = G;
  out++;
  out->s = G;
  out->e = H;
  out++;
  out->s = H;
  out->e = E;
  out++;

  out->s = A;
  out->e = E;
  out++;
  out->s = B;
  out->e = F;
  out++;
  out->s = C;
  out->e = G;
  out++;
  out->s = D;
  out->e = H;
}

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
