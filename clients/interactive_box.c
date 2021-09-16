#define _GNU_SOURCE 1
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-util.h>

#include "helper.h"
#include "z11-client-protocol.h"
#include "z11-input-client-protocol.h"
#include "z11-opengl-client-protocol.h"

typedef struct {
  Point p1, p2, p3;
} Triangle;

struct render_info {
  struct z11_virtual_object *virtual_object;
  Line *shm_line_data;
  Triangle *shm_triangle_data;
  struct z11_opengl_vertex_buffer *line_vertex_buffer;
  struct wl_raw_buffer *line_buffer;
  struct z11_opengl_vertex_buffer *triangle_vertex_buffer;
  struct wl_raw_buffer *triangle_buffer;
  int32_t x, y, z;
  double theta;
  double del_theta;
};

void render(struct render_info *info);
static void paint_plane(Triangle *out, float x, float y, float z, float theta);
static void paint_frame(Line *out, float x, float y, float z, float theta);
const char *vertex_shader;
const char *red_fragment_shader;
const char *orange_fragment_shader;

static void callback_done(void *data, struct wl_callback *callback,
                          uint32_t callback_data)
{
  (void)callback_data;
  struct render_info *info = data;
  wl_callback_destroy(callback);
  render(info);
}

static const struct wl_callback_listener z_callback_listener = {
    .done = callback_done,
};

void render(struct render_info *info)
{
  struct wl_callback *cb;
  paint_frame(info->shm_line_data, info->x, info->y, info->z, info->theta);
  paint_plane(info->shm_triangle_data, info->x, info->y, info->z, info->theta);
  z11_opengl_vertex_buffer_attach(info->line_vertex_buffer, info->line_buffer,
                                  sizeof(Point));
  z11_opengl_vertex_buffer_attach(info->triangle_vertex_buffer,
                                  info->triangle_buffer, sizeof(Point));
  cb = z11_virtual_object_frame(info->virtual_object);
  wl_callback_add_listener(cb, &z_callback_listener, info);
  z11_virtual_object_commit(info->virtual_object);
  info->theta += info->del_theta;
}

static void _shm_format(void *data, struct wl_shm *wl_shm, uint32_t format) {}

static struct wl_shm_listener _shm_listener = {.format = _shm_format};

static void ray_handle_motion(void *data, struct z11_ray *ray, uint32_t a,
                              wl_fixed_t surface_x, wl_fixed_t surface_y)
{
  fprintf(stderr, "enter!!(dx: %d, dy: %d)\n\n", surface_x, surface_y);
}

static const struct z11_ray_listener ray_listener = {
    .motion = ray_handle_motion,
};

static void handle_seat_capabilities(void *data, struct z11_seat *seat,
                                     uint32_t capability)
{
  // /home/shierote/weston/clients/ivi-shell-user-interface.c #520 参考に実装
  // CAPABILITY見て使うデバイス指定できるようにする
  // surface componentにenterした時にそのイベントだけ送るようにする。
  if (capability & Z11_SEAT_CAPABILITY_RAY) {
    struct z11_ray *ray = NULL;
    ray = z11_seat_get_ray(seat);
    z11_ray_add_listener(ray, &ray_listener, data);
  }
}

struct z11_seat_listener seat_listener = {.capability =
                                              handle_seat_capabilities};

static void global_registry_handler(void *data, struct wl_registry *registry,
                                    uint32_t id, const char *interface,
                                    uint32_t version)
{
  struct z11_global *global = data;

  if (strcmp(interface, "z11_compositor") == 0) {
    global->compositor =
        wl_registry_bind(registry, id, &z11_compositor_interface, version);
  } else if (strcmp(interface, "wl_shm") == 0) {
    global->shm = wl_registry_bind(registry, id, &wl_shm_interface, 1);
    wl_shm_add_listener(global->shm, &_shm_listener, NULL);
  } else if (strcmp(interface, "z11_seat") == 0) {
    global->seat = wl_registry_bind(registry, id, &z11_seat_interface, 1);
    z11_seat_add_listener(global->seat, &seat_listener, NULL);
  } else if (strcmp(interface, "z11_opengl") == 0) {
    global->gl = wl_registry_bind(registry, id, &z11_opengl_interface, 1);
  } else if (strcmp(interface, "z11_opengl_render_component_manager") == 0) {
    global->render_component_manager = wl_registry_bind(
        registry, id, &z11_opengl_render_component_manager_interface, 1);
  }
}

static void global_registry_remover(void *data, struct wl_registry *registry,
                                    uint32_t id)
{}

static const struct wl_registry_listener registry_listener = {
    global_registry_handler,
    global_registry_remover,
};

struct z11_global *create_global()
{
  struct z11_global *global;
  struct wl_display *display;
  struct wl_registry *registry;

  const char *socket = "z11-0";

  global = malloc(sizeof *global);
  if (global == NULL) {
    fprintf(stderr, "Fail to allocate memory\n");
    goto no_mem_global;
  }

  global->compositor = NULL;
  global->shm = NULL;
  global->seat = NULL;
  global->gl = NULL;
  global->render_component_manager = NULL;

  display = wl_display_connect(socket);
  if (display == NULL) {
    fprintf(stderr, "Can't connect to display\n");
    goto fail_to_connect;
  }
  global->display = display;
  fprintf(stderr, "Connected to display\n");

  registry = wl_display_get_registry(display);

  wl_registry_add_listener(registry, &registry_listener, global);

  wl_display_dispatch(display);
  wl_display_roundtrip(display);

  assert(global->compositor && global->shm && global->seat && global->gl &&
         global->render_component_manager);

  return global;

fail_to_connect:
  free(global);

no_mem_global:
  return NULL;
}

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

  struct z11_global *global = create_global();

  // prepare shared memory
  int size_of_lines = sizeof(Line) * 12;
  int size_of_triangles = sizeof(Triangle) * 2;
  int fd = create_shared_fd(size_of_lines + size_of_triangles);
  void *shm_data = mmap(NULL, size_of_lines + size_of_triangles,
                        PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
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
  struct wl_shm_pool *pool =
      wl_shm_create_pool(global->shm, fd, size_of_lines + size_of_triangles);
  struct wl_raw_buffer *line_buffer =
      wl_shm_pool_create_raw_buffer(pool, 0, size_of_lines);
  struct wl_raw_buffer *triangle_buffer =
      wl_shm_pool_create_raw_buffer(pool, size_of_lines, size_of_triangles);
  wl_shm_pool_destroy(pool);

  // prepare vertex buffers
  struct z11_opengl_vertex_buffer *line_vertex_buffer =
      z11_opengl_create_vertex_buffer(global->gl);
  struct z11_opengl_vertex_buffer *triangle_vertex_buffer =
      z11_opengl_create_vertex_buffer(global->gl);

  // prepare shaders
  struct z11_opengl_shader_program *frame_shader_program =
      z11_opengl_create_shader_program(global->gl, vertex_shader,
                                       red_fragment_shader);

  struct z11_opengl_shader_program *plane_shader_program =
      z11_opengl_create_shader_program(global->gl, vertex_shader,
                                       orange_fragment_shader);

  // prepare virtual object
  struct z11_virtual_object *virtual_object =
      z11_compositor_create_virtual_object(global->compositor);

  // prepare render component
  struct z11_opengl_render_component *frame_render_component =
      z11_opengl_render_component_manager_create_opengl_render_component(
          global->render_component_manager, virtual_object);
  z11_opengl_render_component_attach_vertex_buffer(frame_render_component,
                                                   line_vertex_buffer);
  z11_opengl_render_component_attach_shader_program(frame_render_component,
                                                    frame_shader_program);
  z11_opengl_render_component_append_vertex_input_attribute(
      frame_render_component, 0,
      Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR3, 0);

  struct z11_opengl_render_component *plane_render_component =
      z11_opengl_render_component_manager_create_opengl_render_component(
          global->render_component_manager, virtual_object);
  z11_opengl_render_component_attach_vertex_buffer(plane_render_component,
                                                   triangle_vertex_buffer);
  z11_opengl_render_component_attach_shader_program(plane_render_component,
                                                    plane_shader_program);
  z11_opengl_render_component_append_vertex_input_attribute(
      plane_render_component, 0,
      Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR3, 0);
  z11_opengl_render_component_set_topology(plane_render_component,
                                           Z11_OPENGL_TOPOLOGY_TRIANGLES);

  // first render
  paint_frame(shm_line_data, x, y, z, theta);
  paint_plane(shm_triangle_data, x, y, z, theta);
  z11_opengl_vertex_buffer_attach(line_vertex_buffer, line_buffer,
                                  sizeof(Point));
  z11_opengl_vertex_buffer_attach(triangle_vertex_buffer, triangle_buffer,
                                  sizeof(Point));
  z11_virtual_object_commit(virtual_object);

  struct render_info info;
  info.virtual_object = virtual_object;
  info.line_vertex_buffer = line_vertex_buffer;
  info.line_buffer = line_buffer;
  info.triangle_vertex_buffer = triangle_vertex_buffer;
  info.triangle_buffer = triangle_buffer;
  info.shm_line_data = shm_line_data;
  info.shm_triangle_data = shm_triangle_data;
  info.x = x;
  info.y = y;
  info.z = z;
  info.theta = theta;
  info.del_theta = del_theta;

  render(&info);

  int ret;
  while (1) {
    while (wl_display_prepare_read(global->display) != 0) {
      if (errno != EAGAIN) break;
      wl_display_dispatch_pending(global->display);
    }
    ret = wl_display_flush(global->display);
    if (ret == -1) break;
    wl_display_read_events(global->display);
    wl_display_dispatch_pending(global->display);
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
