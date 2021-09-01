#include "input.h"

#include <GL/glew.h>
#include <libzazen.h>
#include <stdio.h>
#include <wayland-server-core.h>

#include "compositor.h"
#include "libinput_device.h"
#include "math.h"
#include "opengl_for_render_state.h"
#include "opengl_render_component_manager.h"
#include "util.h"

int zazen_input_init(struct wl_event_loop* loop,
                     struct zazen_opengl_render_component_manager* render_component_manager)
{
  struct zazen_opengl_render_item* render_item = zazen_opengl_render_item_create(render_component_manager);

  Line* ray;
  ray = zalloc(sizeof(Line));
  ray->begin = (Point){2, -2, 5};
  ray->end = (Point){0, 10, 10};
  render_item->vertex_buffer_data = (void*)ray;

  render_item->back_state.vertex_buffer_size = sizeof(Line);
  render_item->back_state.vertex_stride = sizeof(Point);

  render_item->vertex_location = 0;
  render_item->vertex_size = 3;
  render_item->vertex_type = GL_FLOAT;
  render_item->vertex_offset = offsetof(Line, begin);

  render_item->back_state.topology_mode = GL_LINES;

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

  libinput_init(loop, render_item);  // TODO: error handling

  return 1;
}

void zazen_input_destroy() { libinput_destroy(); }
