#include "internal.h"
#include "z11-server-protocol.h"

struct z_render_block_state {
  struct z_gl_vertex_buffer* vertex_buffer; /** nullable */
  struct wl_listener vertex_buffer_destroy_listener;
  struct z_gl_shader_program* shader_program;
  struct wl_listener shader_program_destroy_listener;
  enum z11_gl_topology topology;
};

static void z_render_block_state_vertex_buffer_destroy_signal_handler(struct wl_listener* listener,
                                                                      void* data)
{
  UNUSED(data);
  struct z_render_block_state* render_block_state;

  render_block_state = wl_container_of(listener, render_block_state, vertex_buffer_destroy_listener);

  render_block_state->vertex_buffer = NULL;
}

static void z_render_block_state_shader_program_destroy_signal_handler(struct wl_listener* listener,
                                                                       void* data)
{
  UNUSED(data);
  struct z_render_block_state* render_block_state;

  render_block_state = wl_container_of(listener, render_block_state, vertex_buffer_destroy_listener);

  render_block_state->shader_program = NULL;
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

  state->topology = Z11_GL_TOPOLOGY_LINES;

  return state;

fail:
  return NULL;
}

void z_render_block_state_destroy(struct z_render_block_state* state)
{
  wl_list_remove(&state->vertex_buffer_destroy_listener.link);
  free(state);
}

/**
 * set state->vertex_buffer NULL when client destroyed the vertex_buffer
 */
void z_render_block_state_attach_vertex_buffer(struct z_render_block_state* state,
                                               struct z_gl_vertex_buffer* vertex_buffer)
{
  wl_list_remove(&state->vertex_buffer_destroy_listener.link);
  wl_signal_add(&vertex_buffer->destroy_signal, &state->vertex_buffer_destroy_listener);

  state->vertex_buffer = vertex_buffer;
}

/**
 * @return nullable
 */
struct z_gl_vertex_buffer* z_render_block_state_get_vertex_buffer(struct z_render_block_state* state)
{
  return state->vertex_buffer;
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

enum z11_gl_topology z_render_block_state_get_topology(struct z_render_block_state* state)
{
  return state->topology;
}

void z_render_block_state_set_topology(struct z_render_block_state* state, enum z11_gl_topology topology)
{
  state->topology = topology;
}
