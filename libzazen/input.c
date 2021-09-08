#include "input.h"

#include <libzazen.h>
#include <string.h>

#include "compositor.h"
#include "libinput_device.h"
#include "opengl_render_component_manager.h"
#include "opengl_render_item.h"
#include "opengl_util.h"
#include "util.h"

typedef struct {
  float x, y, z;
} Point;

typedef struct {
  Point begin, end;
} Line;

typedef struct {
  Point p1, p2, p3;
} Triangle;

void notify_motion(struct zazen_seat* seat, struct zazen_pointer_motion_event* event)
{
  seat->pointer->grab->interface->motion(seat->pointer->grab, event);
}

static void grab_pointer_button(struct zazen_pointer_grab* grab, const struct timespec* time, uint32_t button,
                                enum wl_pointer_button_state state)
{
  UNUSED(grab);
  UNUSED(time);
  UNUSED(button);
  UNUSED(state);
  // TODO
}

static void grab_pointer_focus(struct zazen_pointer_grab* grab)
{
  UNUSED(grab);
  // TODO
}

static void grab_pointer_motion(struct zazen_pointer_grab* grab, struct zazen_pointer_motion_event* event)
{
  struct zazen_opengl_render_item* render_item = grab->pointer->render_item;
  Line* ray = (Line*)render_item->vertex_buffer_data;
  ray->end.x += event->dx / 10;
  ray->end.y += -event->dy / 10;
  zazen_opengl_render_item_commit(render_item);
}

static void grab_pointer_axis(struct zazen_pointer_grab* grab, const struct timespec* time,
                              struct zazen_pointer_axis_event* event)
{
  UNUSED(grab);
  UNUSED(time);
  UNUSED(event);
  // TODO
}

static void grab_pointer_axis_source(struct zazen_pointer_grab* grab, enum wl_pointer_axis_source source)
{
  UNUSED(grab);
  UNUSED(source);
  // TODO
}

static void grab_pointer_frame(struct zazen_pointer_grab* grab)
{
  UNUSED(grab);
  // TODO
}

static void grab_pointer_cancel(struct zazen_pointer_grab* grab)
{
  UNUSED(grab);
  // TODO
}

static const struct zazen_pointer_grab_interface default_pointer_grab_interface = {
    .focus = grab_pointer_focus,
    .motion = grab_pointer_motion,
    .button = grab_pointer_button,
    .axis = grab_pointer_axis,
    .axis_source = grab_pointer_axis_source,
    .frame = grab_pointer_frame,
    .cancel = grab_pointer_cancel,
};

void zazen_pointer_set_default_grab(struct zazen_pointer* pointer,
                                    const struct zazen_pointer_grab_interface* interface)
{
  pointer->default_grab.interface = interface;
}

static struct zazen_opengl_render_item* zazen_pointer_render_item_create(struct zazen_seat* seat)
{
  struct zazen_opengl_render_item* render_item;
  Line* ray;

  render_item = zazen_opengl_render_item_create(seat->render_component_manager);

  ray = zalloc(sizeof(Line));
  ray->begin = (Point){2, -2, 5};
  ray->end = (Point){0, 10, 10};
  render_item->vertex_buffer_data = (void*)ray;

  render_item->vertex_buffer_size = sizeof(Line);
  render_item->vertex_buffer_stride = sizeof(Point);

  struct gl_vertex_input_attribute* input_attribute;

  input_attribute = wl_array_add(&render_item->vertex_input_attributes, sizeof *input_attribute);

  input_attribute->location = 0;
  input_attribute->format = Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR3;
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

static struct zazen_pointer* zazen_pointer_create(struct zazen_seat* seat)
{
  struct zazen_pointer* pointer;

  pointer = zalloc(sizeof *pointer);
  if (pointer == NULL) return NULL;

  zazen_pointer_set_default_grab(pointer, &default_pointer_grab_interface);
  pointer->default_grab.pointer = pointer;
  pointer->grab = &pointer->default_grab;

  pointer->render_item = zazen_pointer_render_item_create(seat);

  return pointer;
}

bool zazen_seat_init_keyboard(struct zazen_seat* seat)
{
  UNUSED(seat);
  return true;
}

bool zazen_seat_init_pointer(struct zazen_seat* seat)
{
  struct zazen_pointer* pointer;

  // TODO: Handle capabilities

  if (seat->pointer) {
    seat->pointer_device_count += 1;
    return true;
  }

  pointer = zazen_pointer_create(seat);
  if (pointer == NULL) {
    zazen_log("Unable to create pointer\n");
    return false;
  }

  seat->pointer = pointer;
  seat->pointer_device_count = 1;
  pointer->seat = seat;

  return true;
}

bool zazen_seat_init(struct zazen_seat* seat,
                     struct zazen_opengl_render_component_manager* render_component_manager,
                     const char* seat_name)
{
  seat->render_component_manager = render_component_manager;
  seat->pointer = NULL;
  seat->keyboard = NULL;
  seat->vr_controller = NULL;
  seat->pointer_device_count = 0;
  seat->keyboard_device_count = 0;
  seat->seat_name = strdup(seat_name);

  return true;
}

static void zazen_pointer_destroy(struct zazen_pointer* pointer)
{
  if (pointer->render_item) zazen_opengl_render_item_destroy(pointer->render_item);

  free(pointer);
}

static void zazen_keyboard_destroy(struct zazen_keyboard* keyboard)
{
  if (keyboard->render_item) zazen_opengl_render_item_destroy(keyboard->render_item);

  free(keyboard);
}

static void zazen_seat_release(struct zazen_seat* seat)
{
  if (seat->pointer) zazen_pointer_destroy(seat->pointer);
  if (seat->keyboard) zazen_keyboard_destroy(seat->keyboard);

  free(seat->seat_name);
}

struct zazen_input* zazen_input_create(struct wl_event_loop* loop,
                                       struct zazen_opengl_render_component_manager* render_component_manager)
{
  struct zazen_seat* seat;
  struct zazen_input* input_backend;

  seat = zalloc(sizeof *seat);
  input_backend = zalloc(sizeof *input_backend);

  if (!zazen_seat_init(seat, render_component_manager, "seat0")) {
    zazen_log("Failed to init zazen seat\n");
    goto out;
  }

  input_backend->seat = seat;

  libinput_init(loop, input_backend);

  return input_backend;

out:
  free(seat);

  return NULL;
}

void zazen_input_destroy(struct zazen_input* input_backend)
{
  libinput_destroy(input_backend);
  zazen_seat_release(input_backend->seat);
  free(input_backend);
}
