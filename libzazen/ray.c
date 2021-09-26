#include "ray.h"

#include "cuboid_window.h"
#include "opengl_render_component_back_state.h"
#include "seat.h"
#include "util.h"
#include "z11-input-server-protocol.h"

static const char* fragment_shader;
static const char* vertex_shader;

static void grab_ray_button(struct zazen_ray_grab* grab,
                            const struct timespec* time, uint32_t button,
                            enum wl_pointer_button_state state)
{
  UNUSED(grab);
  UNUSED(time);
  UNUSED(button);
  UNUSED(state);
  // TODO: send event to client
}

static void grab_ray_focus(struct zazen_ray_grab* grab)
{
  UNUSED(grab);
  // TODO: send event to client
}

void zazen_ray_notify_motion(struct zazen_ray* ray,
                             struct zazen_ray_motion_event* event)
{
  ray->grab.interface->motion(&ray->grab, event);
}

static void grab_ray_motion(struct zazen_ray_grab* grab,
                            struct zazen_ray_motion_event* event)
{
  glm_vec3_add(grab->ray->line.begin, event->begin_delta,
               grab->ray->line.begin);
  glm_vec3_add(grab->ray->line.end, event->end_delta, grab->ray->line.end);

  zazen_opengl_render_item_set_vertex_buffer(grab->ray->render_item,
                                             (void*)&grab->ray->line,
                                             sizeof(Line), sizeof(vec3));

  zazen_opengl_render_item_commit(grab->ray->render_item);

  // TODO: send event to client
}

static void grab_ray_axis(struct zazen_ray_grab* grab,
                          const struct timespec* time,
                          struct zazen_ray_axis_event* event)
{
  UNUSED(grab);
  UNUSED(time);
  UNUSED(event);
  // TODO: send event to client
}

static void grab_ray_axis_source(struct zazen_ray_grab* grab,
                                 enum wl_pointer_axis_source source)
{
  UNUSED(grab);
  UNUSED(source);
  // TODO: send event to client
}

static void grab_ray_frame(struct zazen_ray_grab* grab)
{
  UNUSED(grab);
  // TODO: send event to client
}

static void grab_ray_cancel(struct zazen_ray_grab* grab)
{
  UNUSED(grab);
  // TODO: send event to client
}

static const struct zazen_ray_grab_interface ray_grab_interface = {
    .focus = grab_ray_focus,
    .motion = grab_ray_motion,
    .button = grab_ray_button,
    .axis = grab_ray_axis,
    .axis_source = grab_ray_axis_source,
    .frame = grab_ray_frame,
    .cancel = grab_ray_cancel,
};

static void zazen_ray_focus_cuboid_window_destroy_handler(
    struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zazen_ray* ray;
  ray = wl_container_of(listener, ray, zazen_cuboid_window_destroy_listener);
  ray->focus_cuboid_window = NULL;
}

void zazen_ray_enter(struct zazen_ray* ray,
                     struct zazen_cuboid_window* cuboid_window)
{
  if (ray->focus_cuboid_window == cuboid_window) return;
  if (ray->focus_cuboid_window != NULL) {
    zazen_cuboid_window_remove_highlight(ray->focus_cuboid_window);
    wl_list_remove(&ray->zazen_cuboid_window_destroy_listener.link);
  }
  zazen_cuboid_window_highlight(cuboid_window);
  wl_signal_add(&cuboid_window->destroy_signal,
                &ray->zazen_cuboid_window_destroy_listener);
  ray->focus_cuboid_window = cuboid_window;
}

struct zazen_ray* zazen_ray_create(struct zazen_seat* seat)
{
  struct zazen_ray* ray;

  ray = zalloc(sizeof *ray);
  if (ray == NULL) return NULL;

  ray->seat = seat;
  ray->focus_cuboid_window = NULL;

  ray->grab.interface = &ray_grab_interface;
  ray->grab.ray = ray;

  wl_list_init(&ray->ray_clients);
  wl_signal_init(&ray->destroy_signal);

  glm_vec3_copy((vec3){2, -2, 5}, ray->line.begin);
  glm_vec3_copy((vec3){0, 10, 10}, ray->line.end);

  ray->render_item =
      zazen_opengl_render_item_create(seat->render_component_manager);
  if (ray->render_item == NULL) goto out;
  zazen_opengl_render_item_set_vertex_buffer(
      ray->render_item, (void*)&ray->line, sizeof(Line), sizeof(vec3));

  zazen_opengl_render_item_set_shader(ray->render_item, vertex_shader,
                                      fragment_shader);

  zazen_opengl_render_item_set_topology(ray->render_item,
                                        Z11_OPENGL_TOPOLOGY_LINES);

  zazen_opengl_render_item_append_vertex_input_attribute(
      ray->render_item, 0,
      Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR3, 0);

  zazen_opengl_render_item_commit(ray->render_item);

  ray->zazen_cuboid_window_destroy_listener.notify =
      zazen_ray_focus_cuboid_window_destroy_handler;
  wl_list_init(&ray->zazen_cuboid_window_destroy_listener.link);

  return ray;

out:
  free(ray);

  return NULL;
}

void zazen_ray_destroy(struct zazen_ray* ray)
{
  wl_signal_emit(&ray->destroy_signal, ray);
  if (ray->render_item) zazen_opengl_render_item_destroy(ray->render_item);
  free(ray);
}

static const char* vertex_shader =
    "#version 410\n"
    "uniform mat4 mvp;\n"
    "layout(location = 0) in vec4 position;\n"
    "layout(location = 1) in vec2 v2UVcoordsIn;\n"
    "layout(location = 2) in vec3 v3NormalIn;\n"
    "out vec2 v2UVcoords;\n"
    "void main()\n"
    "{\n"
    "  v2UVcoords = v2UVcoordsIn;\n"
    "  gl_Position = mvp * position;\n"
    "}\n";

static const char* fragment_shader =
    "#version 410 core\n"
    "in vec2 v2UVcoords;\n"
    "out vec4 outputColor;\n"
    "void main()\n"
    "{\n"
    "  outputColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
    "}\n";
