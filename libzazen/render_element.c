#include <GL/glew.h>
#include <wayland-server.h>

#include "internal.h"
#include "z11-server-protocol.h"

struct zazen_render_element {
  struct zazen_compositor* compositor;
  struct wl_list link;
  GLuint vertex_array_object;
  struct zazen_render_element_state* current_state;
  struct zazen_render_element_state* next_state;
};

void zazen_render_element_destroy(struct zazen_render_element* render_element);

static void zazen_render_element_handle_destroy(struct wl_resource* resource)
{
  struct zazen_render_element* render_element = wl_resource_get_user_data(resource);

  zazen_render_element_destroy(render_element);
}

static void zazen_render_element_protocol_destroy(struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void zazen_render_element_protocol_attach_vertex_buffer(struct wl_client* client,
                                                               struct wl_resource* resource,
                                                               struct wl_resource* vertex_buffer_resource,
                                                               uint32_t vertex_stride)
{
  UNUSED(client);
  struct zazen_render_element* render_element = wl_resource_get_user_data(resource);
  struct zazen_gl_vertex_buffer* vertex_buffer = wl_resource_get_user_data(vertex_buffer_resource);

  zazen_render_element_state_attach_vertex_buffer(render_element->next_state, vertex_buffer, vertex_stride);
}

static void zazen_render_element_protocol_attach_shader_program(struct wl_client* client,
                                                                struct wl_resource* resource,
                                                                struct wl_resource* shader_program_resource)
{
  UNUSED(client);
  struct zazen_render_element* render_element = wl_resource_get_user_data(resource);
  struct zazen_gl_shader_program* shader_program = wl_resource_get_user_data(shader_program_resource);

  zazen_render_element_state_attach_shader_program(render_element->next_state, shader_program);
}

static void zazen_render_element_protocol_attach_texture_2d(struct wl_client* client,
                                                            struct wl_resource* resource,
                                                            struct wl_resource* texture_2d_resource)
{
  UNUSED(client);
  struct zazen_render_element* render_element = wl_resource_get_user_data(resource);
  struct zazen_gl_texture_2d* texture_2d = wl_resource_get_user_data(texture_2d_resource);

  zazen_render_element_state_attach_texture_2d(render_element->next_state, texture_2d);
}

static void zazen_render_element_protocol_append_vertex_input_attribute(
    struct wl_client* client, struct wl_resource* resource, uint32_t location,
    enum z11_gl_vertex_input_attribute_format format, uint32_t offset)
{
  UNUSED(client);
  struct zazen_render_element* render_element = wl_resource_get_user_data(resource);
  zazen_render_element_state_append_vertex_input_attribute(render_element->next_state, location, format,
                                                           offset);
}

static void zazen_render_element_protocol_set_topology(struct wl_client* client, struct wl_resource* resource,
                                                       enum z11_gl_topology topology)
{
  UNUSED(client);
  struct zazen_render_element* render_element = wl_resource_get_user_data(resource);

  zazen_render_element_state_set_topology(render_element->next_state, topology);
}

static GLuint zazen_render_element_get_size_from_attribute_format(
    enum z11_gl_vertex_input_attribute_format format)
{
  switch (format) {
    case Z11_GL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_SCALAR:
      return 1;
    case Z11_GL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR2:
      return 2;
    case Z11_GL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR3:
      return 3;
    case Z11_GL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR4:
      return 4;
    default:
      return 0;
  }
}

static GLenum zazen_render_element_get_type_from_attribute_format(
    enum z11_gl_vertex_input_attribute_format format)
{
  switch (format) {
    case Z11_GL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_SCALAR:
    case Z11_GL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR2:
    case Z11_GL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR3:
    case Z11_GL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR4:
      return GL_FLOAT;
    default:
      return GL_FLOAT;
  }
}

static void zazen_render_element_protocol_commit(struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);
  struct zazen_render_element* render_element = wl_resource_get_user_data(resource);

  zazen_render_element_state_destroy(render_element->current_state);
  render_element->current_state = render_element->next_state;
  render_element->next_state = zazen_render_element_state_create();

  struct zazen_gl_vertex_buffer* vertex_buffer =
      zazen_render_element_state_get_vertex_buffer(render_element->current_state);
  struct wl_array* vertex_input_attributes =
      zazen_render_element_state_get_vertex_input_attributes(render_element->current_state);
  struct zazen_gl_vertex_input_attribute* vertex_input_attribute;

  if (vertex_buffer == NULL) return;

  uint32_t vertex_stride = zazen_render_element_state_get_vertex_stride(render_element->current_state);
  glBindVertexArray(render_element->vertex_array_object);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer->id);
  wl_array_for_each(vertex_input_attribute, vertex_input_attributes)
  {
    GLint size = zazen_render_element_get_size_from_attribute_format(vertex_input_attribute->format);
    GLenum type = zazen_render_element_get_type_from_attribute_format(vertex_input_attribute->format);
    glEnableVertexAttribArray(vertex_input_attribute->location);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-conversion"
    glVertexAttribPointer(vertex_input_attribute->location, size, type, GL_FALSE, vertex_stride,
                          vertex_input_attribute->offset);
  }
#pragma GCC diagnostic pop

  glBindVertexArray(0);
}

static const struct z11_render_element_interface zazen_render_element_interface = {
    .destroy = zazen_render_element_protocol_destroy,
    .attach_vertex_buffer = zazen_render_element_protocol_attach_vertex_buffer,
    .attach_shader_program = zazen_render_element_protocol_attach_shader_program,
    .attach_texture_2d = zazen_render_element_protocol_attach_texture_2d,
    .append_vertex_input_attribute = zazen_render_element_protocol_append_vertex_input_attribute,
    .set_topology = zazen_render_element_protocol_set_topology,
    .commit = zazen_render_element_protocol_commit,
};

static GLenum zazen_render_element_get_current_state_opengl_topology_mode(
    struct zazen_render_element* render_element)
{
  enum z11_gl_topology topology = zazen_render_element_state_get_topology(render_element->current_state);

  switch (topology) {
    case Z11_GL_TOPOLOGY_POINTS:
      return GL_POINTS;
    case Z11_GL_TOPOLOGY_LINES:
      return GL_LINES;
    case Z11_GL_TOPOLOGY_LINE_STRIP:
      return GL_LINE_STRIP;
    case Z11_GL_TOPOLOGY_TRIANGLES:
      return GL_TRIANGLES;
    case Z11_GL_TOPOLOGY_TRIANGLE_STRIP:
      return GL_TRIANGLE_STRIP;
    case Z11_GL_TOPOLOGY_TRIANGLE_FAN:
      return GL_TRIANGLE_FAN;
    default:
      return GL_LINES;
  }
}

struct wl_list* zazen_render_element_get_link(struct zazen_render_element* render_element)
{
  return &render_element->link;
}

/**
 * call
 *
 * glUseProgram(0);
 * glBindTexture(GL_TEXTURE_2D, 0)
 * glBindVertexArray(0);
 *
 * after all render elements are processed.
 */
void zazen_render_element_draw(struct zazen_render_element* render_element,
                               const float* view_projection_matrix)
{
  struct zazen_gl_vertex_buffer* vertex_buffer =
      zazen_render_element_state_get_vertex_buffer(render_element->current_state);
  struct zazen_gl_shader_program* shader_program =
      zazen_render_element_state_get_shader_program(render_element->current_state);
  struct zazen_gl_texture_2d* texture_2d =
      zazen_render_element_state_get_texture_2d(render_element->current_state);

  if (vertex_buffer == NULL || shader_program == NULL) return;

  GLenum mode = zazen_render_element_get_current_state_opengl_topology_mode(render_element);
  GLuint program = zazen_gl_shader_program_get_id(shader_program);

  glUseProgram(program);
  GLint view_projection_matrix_location =
      glGetUniformLocation(program, "matrix");  // TODO: Be customizable by client
  glUniformMatrix4fv(view_projection_matrix_location, 1, GL_FALSE, view_projection_matrix);
  glBindVertexArray(render_element->vertex_array_object);
  if (texture_2d != NULL) {
    glBindTexture(GL_TEXTURE_2D, zazen_gl_texture_2d_get_id(texture_2d));
  } else {
    glBindTexture(GL_TEXTURE_2D, 0);
  }
  glDrawArrays(mode, 0, vertex_buffer->size / (sizeof(float) * 3));
}

struct zazen_render_element* zazen_render_element_from_link(struct wl_list* list)
{
  struct zazen_render_element* render_element;
  render_element = wl_container_of(list, render_element, link);
  return render_element;
}

struct zazen_render_element* zazen_render_element_create(struct wl_client* client, uint32_t id,
                                                         struct zazen_compositor* compositor)
{
  struct zazen_render_element* render_element;
  struct wl_resource* resource;

  render_element = zalloc(sizeof *render_element);
  if (render_element == NULL) goto no_mem_render_element;

  render_element->current_state = zazen_render_element_state_create();
  if (render_element->current_state == NULL) goto no_mem_current_state;

  render_element->next_state = zazen_render_element_state_create();
  if (render_element->next_state == NULL) goto no_mem_next_state;

  resource = wl_resource_create(client, &z11_render_element_interface, 1, id);
  if (resource == NULL) goto no_mem_resource;

  render_element->compositor = compositor;

  wl_list_init(&render_element->link);

  glGenVertexArrays(1, &render_element->vertex_array_object);

  wl_resource_set_implementation(resource, &zazen_render_element_interface, render_element,
                                 zazen_render_element_handle_destroy);

  zazen_compositor_append_render_element(compositor, render_element);

  return render_element;

no_mem_resource:
  free(render_element->next_state);

no_mem_next_state:
  free(render_element->current_state);

no_mem_current_state:
  free(render_element);

no_mem_render_element:
  wl_client_post_no_memory(client);
  return NULL;
}

void zazen_render_element_destroy(struct zazen_render_element* render_element)
{
  wl_list_remove(&render_element->link);
  zazen_render_element_state_destroy(render_element->current_state);
  zazen_render_element_state_destroy(render_element->next_state);
  glDeleteVertexArrays(1, &render_element->vertex_array_object);
  free(render_element);
}
