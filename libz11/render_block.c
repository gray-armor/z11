#include <GL/glew.h>
#include <wayland-server.h>

#include "internal.h"
#include "z11-server-protocol.h"

struct z_render_block {
  struct z_compositor* compositor;
  struct wl_list link;
  GLuint vertex_array_object;
  struct z_render_block_state* current_state;
  struct z_render_block_state* next_state;
};

void z_render_block_destroy(struct z_render_block* render_block);

static void z_render_block_handle_destroy(struct wl_resource* resource)
{
  struct z_render_block* render_block = wl_resource_get_user_data(resource);

  z_render_block_destroy(render_block);
}

static void z_render_block_protocol_destroy(struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void z_render_block_protocol_attach_vertex_buffer(struct wl_client* client,
                                                         struct wl_resource* resource,
                                                         struct wl_resource* vertex_buffer_resource,
                                                         uint32_t vertex_stride)
{
  UNUSED(client);
  struct z_render_block* render_block = wl_resource_get_user_data(resource);
  struct z_gl_vertex_buffer* vertex_buffer = wl_resource_get_user_data(vertex_buffer_resource);

  z_render_block_state_attach_vertex_buffer(render_block->next_state, vertex_buffer, vertex_stride);
}

static void z_render_block_protocol_attach_shader_program(struct wl_client* client,
                                                          struct wl_resource* resource,
                                                          struct wl_resource* shader_program_resource)
{
  UNUSED(client);
  struct z_render_block* render_block = wl_resource_get_user_data(resource);
  struct z_gl_shader_program* shader_program = wl_resource_get_user_data(shader_program_resource);

  z_render_block_state_attach_shader_program(render_block->next_state, shader_program);
}

static void z_render_block_protocol_attach_texture_2d(struct wl_client* client, struct wl_resource* resource,
                                                      struct wl_resource* texture_2d_resource)
{
  UNUSED(client);
  struct z_render_block* render_block = wl_resource_get_user_data(resource);
  struct z_gl_texture_2d* texture_2d = wl_resource_get_user_data(texture_2d_resource);

  z_render_block_state_attach_texture_2d(render_block->next_state, texture_2d);
}

static void z_render_block_protocol_append_vertex_input_attribute(
    struct wl_client* client, struct wl_resource* resource, uint32_t location,
    enum z11_gl_vertex_input_attribute_format format, uint32_t offset)
{
  UNUSED(client);
  struct z_render_block* render_block = wl_resource_get_user_data(resource);
  z_render_block_state_append_vertex_input_attribute(render_block->next_state, location, format, offset);
}

static void z_render_block_protocol_set_topology(struct wl_client* client, struct wl_resource* resource,
                                                 enum z11_gl_topology topology)
{
  UNUSED(client);
  struct z_render_block* render_block = wl_resource_get_user_data(resource);

  z_render_block_state_set_topology(render_block->next_state, topology);
}

static GLuint z_render_block_get_size_from_attribute_format(enum z11_gl_vertex_input_attribute_format format)
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

static GLenum z_render_block_get_type_from_attribute_format(enum z11_gl_vertex_input_attribute_format format)
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

static void z_render_block_protocol_commit(struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);
  struct z_render_block* render_block = wl_resource_get_user_data(resource);

  z_render_block_state_destroy(render_block->current_state);
  render_block->current_state = render_block->next_state;
  render_block->next_state = z_render_block_state_create();

  struct z_gl_vertex_buffer* vertex_buffer =
      z_render_block_state_get_vertex_buffer(render_block->current_state);
  struct wl_array* vertex_input_attributes =
      z_render_block_state_get_vertex_input_attributes(render_block->current_state);
  struct z_gl_vertex_input_attribute* vertex_input_attribute;

  if (vertex_buffer == NULL) return;

  uint32_t vertex_stride = z_render_block_state_get_vertex_stride(render_block->current_state);
  glBindVertexArray(render_block->vertex_array_object);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer->id);
  wl_array_for_each(vertex_input_attribute, vertex_input_attributes)
  {
    GLint size = z_render_block_get_size_from_attribute_format(vertex_input_attribute->format);
    GLenum type = z_render_block_get_type_from_attribute_format(vertex_input_attribute->format);
    glEnableVertexAttribArray(vertex_input_attribute->location);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-conversion"
    glVertexAttribPointer(vertex_input_attribute->location, size, type, GL_FALSE, vertex_stride,
                          vertex_input_attribute->offset);
  }
#pragma GCC diagnostic pop

  glBindVertexArray(0);
}

static const struct z11_render_block_interface z_render_block_interface = {
    .destroy = z_render_block_protocol_destroy,
    .attach_vertex_buffer = z_render_block_protocol_attach_vertex_buffer,
    .attach_shader_program = z_render_block_protocol_attach_shader_program,
    .attach_texture_2d = z_render_block_protocol_attach_texture_2d,
    .append_vertex_input_attribute = z_render_block_protocol_append_vertex_input_attribute,
    .set_topology = z_render_block_protocol_set_topology,
    .commit = z_render_block_protocol_commit,
};

static GLenum z_render_block_get_current_state_opengl_topology_mode(struct z_render_block* render_block)
{
  enum z11_gl_topology topology = z_render_block_state_get_topology(render_block->current_state);

  switch (topology) {
    case Z11_GL_TOPOLOGY_LINES:
      return GL_LINES;
    case Z11_GL_TOPOLOGY_TRIANGLES:
      return GL_TRIANGLES;
    default:
      return GL_LINES;
  }
}

struct wl_list* z_render_block_get_link(struct z_render_block* render_block) { return &render_block->link; }

/**
 * call
 *
 * glUseProgram(0);
 * glBindTexture(GL_TEXTURE_2D, 0)
 * glBindVertexArray(0);
 *
 * after all render blocks are processed.
 */
void z_render_block_draw(struct z_render_block* render_block, const float* view_projection_matrix)
{
  struct z_gl_vertex_buffer* vertex_buffer =
      z_render_block_state_get_vertex_buffer(render_block->current_state);
  struct z_gl_shader_program* shader_program =
      z_render_block_state_get_shader_program(render_block->current_state);
  struct z_gl_texture_2d* texture_2d = z_render_block_state_get_texture_2d(render_block->current_state);

  if (vertex_buffer == NULL || shader_program == NULL) return;

  GLenum mode = z_render_block_get_current_state_opengl_topology_mode(render_block);
  GLuint program = z_gl_shader_program_get_id(shader_program);

  glUseProgram(program);
  GLint view_projection_matrix_location =
      glGetUniformLocation(program, "matrix");  // TODO: Be customizable by client
  glUniformMatrix4fv(view_projection_matrix_location, 1, GL_FALSE, view_projection_matrix);
  glBindVertexArray(render_block->vertex_array_object);
  if (texture_2d != NULL) {
    glBindTexture(GL_TEXTURE_2D, z_gl_texture_2d_get_id(texture_2d));
  } else {
    glBindTexture(GL_TEXTURE_2D, 0);
  }
  glDrawArrays(mode, 0, vertex_buffer->size / (sizeof(float) * 3));
}

struct z_render_block* z_render_block_from_link(struct wl_list* list)
{
  struct z_render_block* render_block;
  render_block = wl_container_of(list, render_block, link);
  return render_block;
}

struct z_render_block* z_render_block_create(struct wl_client* client, uint32_t id,
                                             struct z_compositor* compositor)
{
  struct z_render_block* render_block;
  struct wl_resource* resource;

  render_block = zalloc(sizeof *render_block);
  if (render_block == NULL) goto no_mem_render_block;

  render_block->current_state = z_render_block_state_create();
  if (render_block->current_state == NULL) goto no_mem_current_state;

  render_block->next_state = z_render_block_state_create();
  if (render_block->next_state == NULL) goto no_mem_next_state;

  resource = wl_resource_create(client, &z11_render_block_interface, 1, id);
  if (resource == NULL) goto no_mem_resource;

  render_block->compositor = compositor;

  wl_list_init(&render_block->link);

  glGenVertexArrays(1, &render_block->vertex_array_object);

  wl_resource_set_implementation(resource, &z_render_block_interface, render_block,
                                 z_render_block_handle_destroy);

  z_compositor_append_render_block(compositor, render_block);

  return render_block;

no_mem_resource:
  free(render_block->next_state);

no_mem_next_state:
  free(render_block->current_state);

no_mem_current_state:
  free(render_block);

no_mem_render_block:
  wl_client_post_no_memory(client);
  return NULL;
}

void z_render_block_destroy(struct z_render_block* render_block)
{
  wl_list_remove(&render_block->link);
  z_render_block_state_destroy(render_block->current_state);
  z_render_block_state_destroy(render_block->next_state);
  glDeleteVertexArrays(1, &render_block->vertex_array_object);
  free(render_block);
}
