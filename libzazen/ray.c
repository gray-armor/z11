#include "ray.h"

#include "opengl_render_component_back_state.h"
#include "seat.h"
#include "util.h"

typedef struct {
  float x, y, z;
} Point;

typedef struct {
  Point begin, end;
} Line;

static void zazen_ray_set_origin(struct zazen_ray* ray, float dx, float dy,
                                 float dz)
{
  Line* ray_line = (Line*)ray->render_item->vertex_buffer_data;

  ray->origin_x += dx;
  ray->origin_y += dy;
  ray->origin_z += dz;

  ray_line->begin = (Point){ray->origin_x, ray->origin_y, ray->origin_z};
}

static void zazen_ray_set_direction(struct zazen_ray* ray, float dx, float dy,
                                    float dz)
{
  Line* ray_line = (Line*)ray->render_item->vertex_buffer_data;
  float direction_size;
  float ray_size = 10;

  ray->direction_x += dx;
  ray->direction_y += dy;
  ray->direction_z += dz;

  direction_size = sqrt(ray->direction_x * ray->direction_x +
                        ray->direction_y * ray->direction_y +
                        ray->direction_z * ray->direction_z);

  if (direction_size > 0) {
    // Convert to unit vector
    ray->direction_x /= direction_size;
    ray->direction_y /= direction_size;
    ray->direction_z /= direction_size;
  }

  ray_line->end = (Point){
      ray->origin_x + ray_size * ray->direction_x,
      ray->origin_y + ray_size * ray->direction_y,
      ray->origin_z + ray_size * ray->direction_z,
  };
}

static struct zazen_opengl_render_item* zazen_ray_render_item_create(
    struct zazen_ray* ray,
    struct zazen_opengl_render_component_manager* render_component_manager)
{
  struct zazen_opengl_render_item* render_item;
  Line* ray_line;
  float ray_size = 10;

  render_item = zazen_opengl_render_item_create(render_component_manager);

  ray_line = zalloc(sizeof(Line));
  ray_line->begin = (Point){
      ray->origin_x,
      ray->origin_y,
      ray->origin_z,
  };

  ray_line->end = (Point){
      ray->origin_x + ray_size * ray->direction_x,
      ray->origin_y + ray_size * ray->direction_y,
      ray->origin_z + ray_size * ray->direction_z,
  };

  render_item->vertex_buffer_data = (void*)ray_line;

  render_item->vertex_buffer_size = sizeof(Line);
  render_item->vertex_buffer_stride = sizeof(Point);

  struct zazen_opengl_render_component_back_state_vertex_input_attribute*
      input_attribute;

  input_attribute = wl_array_add(&render_item->vertex_input_attributes,
                                 sizeof *input_attribute);

  input_attribute->location = 0;
  input_attribute->format =
      Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR3;
  input_attribute->offset = offsetof(Line, begin);

  render_item->topology = Z11_OPENGL_TOPOLOGY_LINES;

  render_item->vertex_shader_source =
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

  render_item->fragment_shader_source =
      "#version 410 core\n"
      "in vec2 v2UVcoords;\n"
      "out vec4 outputColor;\n"
      "void main()\n"
      "{\n"
      "  outputColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
      "}\n";

  zazen_opengl_render_item_commit(render_item);

  return render_item;
}

static void grab_ray_button(struct zazen_ray_grab* grab,
                            const struct timespec* time, uint32_t button,
                            enum wl_pointer_button_state state)
{
  UNUSED(grab);
  UNUSED(time);
  UNUSED(button);
  UNUSED(state);
  // TODO
}

static void grab_ray_focus(struct zazen_ray_grab* grab)
{
  UNUSED(grab);
  // TODO
}

void zazen_ray_notify_motion(struct zazen_ray* ray,
                             struct zazen_ray_motion_event* event)
{
  ray->grab.interface->motion(&ray->grab, event);
}

static void grab_ray_motion(struct zazen_ray_grab* grab,
                            struct zazen_ray_motion_event* event)
{
  zazen_ray_set_origin(grab->ray, event->origin_dx, event->origin_dy,
                       event->origin_dz);

  zazen_ray_set_direction(grab->ray, event->direction_dx, event->direction_dy,
                          event->direction_dz);

  zazen_opengl_render_item_commit(grab->ray->render_item);
}

static void grab_ray_axis(struct zazen_ray_grab* grab,
                          const struct timespec* time,
                          struct zazen_ray_axis_event* event)
{
  UNUSED(grab);
  UNUSED(time);
  UNUSED(event);
  // TODO
}

static void grab_ray_axis_source(struct zazen_ray_grab* grab,
                                 enum wl_pointer_axis_source source)
{
  UNUSED(grab);
  UNUSED(source);
  // TODO
}

static void grab_ray_frame(struct zazen_ray_grab* grab)
{
  UNUSED(grab);
  // TODO
}

static void grab_ray_cancel(struct zazen_ray_grab* grab)
{
  UNUSED(grab);
  // TODO
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

struct zazen_ray* zazen_ray_create(struct zazen_seat* seat)
{
  struct zazen_ray* ray;

  ray = zalloc(sizeof *ray);

  ray->seat = seat;

  ray->grab.interface = &ray_grab_interface;
  ray->grab.ray = ray;

  ray->origin_x = 2;
  ray->origin_y = -2;
  ray->origin_z = 5;

  ray->direction_x = 0;
  ray->direction_y = 1;
  ray->direction_z = 0;

  ray->render_item =
      zazen_ray_render_item_create(ray, seat->render_component_manager);
  if (ray->render_item == NULL) {
    zazen_log("Failed to create render_item\n");
    goto out;
  }

  return ray;

out:
  free(ray);

  return NULL;
}

void zazen_ray_destroy(struct zazen_ray* ray)
{
  if (ray->render_item) zazen_opengl_render_item_destroy(ray->render_item);

  free(ray);
}
