#include "internal.h"
#include "z11-server-protocol.h"

struct z_render_block_state {
  struct z_gl_vertex_buffer* vertex_buffer; /** nullable */
  struct wl_listener vertex_buffer_destroy_listener;
  uint32_t vertex_stride;
  struct z_gl_shader_program* shader_program;
  struct wl_listener shader_program_destroy_listener;
  struct z_gl_texture_2d* texture_2d;
  struct wl_listener texture_2d_destroy_listener;
  struct wl_array vertex_input_attributes;
  enum z11_gl_topology topology;
};

static void z_render_block_state_vertex_buffer_destroy_signal_handler(struct wl_listener* listener,
                                                                      void* data)
{
  UNUSED(data);
  struct z_render_block_state* render_block_state;

  render_block_state = wl_container_of(listener, render_block_state, vertex_buffer_destroy_listener);

  wl_list_remove(&render_block_state->vertex_buffer_destroy_listener.link);
  wl_list_init(&render_block_state->vertex_buffer_destroy_listener.link);
  render_block_state->vertex_buffer = NULL;
  render_block_state->vertex_stride = 0;
}

static void z_render_block_state_shader_program_destroy_signal_handler(struct wl_listener* listener,
                                                                       void* data)
{
  UNUSED(data);
  struct z_render_block_state* render_block_state;

  render_block_state = wl_container_of(listener, render_block_state, shader_program_destroy_listener);

  wl_list_remove(&render_block_state->shader_program_destroy_listener.link);
  wl_list_init(&render_block_state->shader_program_destroy_listener.link);
  render_block_state->shader_program = NULL;
}

static void z_render_block_state_texture_2d_destroy_signal_handler(struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct z_render_block_state* render_block_state;

  render_block_state = wl_container_of(listener, render_block_state, texture_2d_destroy_listener);

  wl_list_remove(&render_block_state->texture_2d_destroy_listener.link);
  wl_list_init(&render_block_state->texture_2d_destroy_listener.link);
  render_block_state->texture_2d = NULL;
}

struct z_render_block_state* z_render_block_state_create()
{
  struct z_render_block_state* state;

  state = zalloc(sizeof *state);
  if (state == NULL) goto fail;

  state->vertex_buffer = NULL;
  wl_list_init(&state->vertex_buffer_destroy_listener.link);
  state->vertex_buffer_destroy_listener.notify = z_render_block_state_vertex_buffer_destroy_signal_handler;

  state->shader_program = NULL;
  wl_list_init(&state->shader_program_destroy_listener.link);
  state->shader_program_destroy_listener.notify = z_render_block_state_shader_program_destroy_signal_handler;

  state->texture_2d = NULL;
  wl_list_init(&state->texture_2d_destroy_listener.link);
  state->texture_2d_destroy_listener.notify = z_render_block_state_texture_2d_destroy_signal_handler;

  wl_array_init(&state->vertex_input_attributes);

  state->topology = Z11_GL_TOPOLOGY_LINES;

  return state;

fail:
  return NULL;
}

void z_render_block_state_destroy(struct z_render_block_state* state)
{
  wl_list_remove(&state->vertex_buffer_destroy_listener.link);
  wl_list_remove(&state->shader_program_destroy_listener.link);
  wl_list_remove(&state->texture_2d_destroy_listener.link);
  wl_array_release(&state->vertex_input_attributes);
  free(state);
}

/**
 * set state->vertex_buffer NULL when client destroyed the vertex_buffer
 */
void z_render_block_state_attach_vertex_buffer(struct z_render_block_state* state,
                                               struct z_gl_vertex_buffer* vertex_buffer,
                                               uint32_t vertex_stride)
{
  wl_list_remove(&state->vertex_buffer_destroy_listener.link);
  wl_signal_add(&vertex_buffer->destroy_signal, &state->vertex_buffer_destroy_listener);

  state->vertex_buffer = vertex_buffer;
  state->vertex_stride = vertex_stride;
}

/**
 * @return nullable
 */
struct z_gl_vertex_buffer* z_render_block_state_get_vertex_buffer(struct z_render_block_state* state)
{
  return state->vertex_buffer;
}

uint32_t z_render_block_state_get_vertex_stride(struct z_render_block_state* state)
{
  return state->vertex_stride;
}

/**
 * set state->shader_program NULL when client destroyed the shader_program
 */
void z_render_block_state_attach_shader_program(struct z_render_block_state* state,
                                                struct z_gl_shader_program* shader_program)
{
  wl_list_remove(&state->shader_program_destroy_listener.link);
  z_gl_shader_program_add_destroy_signal_handler(shader_program, &state->shader_program_destroy_listener);

  state->shader_program = shader_program;
}

/**
 * @return nullable
 */
struct z_gl_shader_program* z_render_block_state_get_shader_program(struct z_render_block_state* state)
{
  return state->shader_program;
}

void z_render_block_state_attach_texture_2d(struct z_render_block_state* state,
                                            struct z_gl_texture_2d* texture_2d)
{
  wl_list_remove(&state->texture_2d_destroy_listener.link);
  z_gl_texture_2d_add_destroy_signal_handler(texture_2d, &state->texture_2d_destroy_listener);

  state->texture_2d = texture_2d;
}

/**
 * @return nullable
 */
struct z_gl_texture_2d* z_render_block_state_get_texture_2d(struct z_render_block_state* state)
{
  return state->texture_2d;
}

void z_render_block_state_append_vertex_input_attribute(struct z_render_block_state* state, uint32_t location,
                                                        enum z11_gl_vertex_input_attribute_format format,
                                                        uint32_t offset)
{
  struct z_gl_vertex_input_attribute* vertex_input_attribute;
  vertex_input_attribute = wl_array_add(&state->vertex_input_attributes, sizeof *vertex_input_attribute);
  vertex_input_attribute->location = location;
  vertex_input_attribute->format = format;
  vertex_input_attribute->offset = offset;
}

struct wl_array* z_render_block_state_get_vertex_input_attributes(struct z_render_block_state* state)
{
  return &state->vertex_input_attributes;
}

enum z11_gl_topology z_render_block_state_get_topology(struct z_render_block_state* state)
{
  return state->topology;
}

void z_render_block_state_set_topology(struct z_render_block_state* state, enum z11_gl_topology topology)
{
  state->topology = topology;
}
