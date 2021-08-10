#include "internal.h"
#include "z11-server-protocol.h"

struct z_render_element_state {
  struct z_week_ref vertex_buffer_ref;
  uint32_t vertex_stride;
  struct z_week_ref shader_program_ref;
  struct z_week_ref texture_2d_ref;
  struct wl_array vertex_input_attributes;
  enum z11_gl_topology topology;
};

static void z_render_element_state_vertex_buffer_on_destroy(struct z_week_ref* ref)
{
  struct z_render_element_state* state;

  state = wl_container_of(ref, state, vertex_buffer_ref);

  state->vertex_stride = 0;
}

struct z_render_element_state* z_render_element_state_create()
{
  struct z_render_element_state* state;

  state = zalloc(sizeof *state);
  if (state == NULL) goto fail;

  z_week_ref_init(&state->vertex_buffer_ref);
  z_week_ref_init(&state->shader_program_ref);
  z_week_ref_init(&state->texture_2d_ref);
  wl_array_init(&state->vertex_input_attributes);

  state->topology = Z11_GL_TOPOLOGY_LINES;

  return state;

fail:
  return NULL;
}

void z_render_element_state_destroy(struct z_render_element_state* state)
{
  z_week_ref_destroy(&state->vertex_buffer_ref);
  z_week_ref_destroy(&state->shader_program_ref);
  z_week_ref_destroy(&state->texture_2d_ref);
  wl_array_release(&state->vertex_input_attributes);
  free(state);
}

/**
 * set state->vertex_buffer NULL when client destroyed the vertex_buffer
 */
void z_render_element_state_attach_vertex_buffer(struct z_render_element_state* state,
                                                 struct z_gl_vertex_buffer* vertex_buffer,
                                                 uint32_t vertex_stride)
{
  z_week_ref_set_data(&state->vertex_buffer_ref, vertex_buffer, &vertex_buffer->destroy_signal,
                      z_render_element_state_vertex_buffer_on_destroy);
  state->vertex_stride = vertex_stride;
}

/**
 * @return nullable
 */
struct z_gl_vertex_buffer* z_render_element_state_get_vertex_buffer(struct z_render_element_state* state)
{
  return state->vertex_buffer_ref.data;
}

uint32_t z_render_element_state_get_vertex_stride(struct z_render_element_state* state)
{
  return state->vertex_stride;
}

/**
 * set state->shader_program NULL when client destroyed the shader_program
 */
void z_render_element_state_attach_shader_program(struct z_render_element_state* state,
                                                  struct z_gl_shader_program* shader_program)
{
  z_week_ref_set_data(&state->shader_program_ref, shader_program,
                      z_gl_shader_program_get_destroy_signal(shader_program), NULL);
}

/**
 * @return nullable
 */
struct z_gl_shader_program* z_render_element_state_get_shader_program(struct z_render_element_state* state)
{
  return state->shader_program_ref.data;
}

void z_render_element_state_attach_texture_2d(struct z_render_element_state* state,
                                              struct z_gl_texture_2d* texture_2d)
{
  z_week_ref_set_data(&state->texture_2d_ref, texture_2d, z_gl_texture_2d_get_destroy_signal(texture_2d),
                      NULL);
}

/**
 * @return nullable
 */
struct z_gl_texture_2d* z_render_element_state_get_texture_2d(struct z_render_element_state* state)
{
  return state->texture_2d_ref.data;
}

void z_render_element_state_append_vertex_input_attribute(struct z_render_element_state* state,
                                                          uint32_t location,
                                                          enum z11_gl_vertex_input_attribute_format format,
                                                          uint32_t offset)
{
  struct z_gl_vertex_input_attribute* vertex_input_attribute;
  vertex_input_attribute = wl_array_add(&state->vertex_input_attributes, sizeof *vertex_input_attribute);
  vertex_input_attribute->location = location;
  vertex_input_attribute->format = format;
  vertex_input_attribute->offset = offset;
}

struct wl_array* z_render_element_state_get_vertex_input_attributes(struct z_render_element_state* state)
{
  return &state->vertex_input_attributes;
}

enum z11_gl_topology z_render_element_state_get_topology(struct z_render_element_state* state)
{
  return state->topology;
}

void z_render_element_state_set_topology(struct z_render_element_state* state, enum z11_gl_topology topology)
{
  state->topology = topology;
}
