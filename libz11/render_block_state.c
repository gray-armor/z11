#include "internal.h"

struct z_render_block_state {
  struct z_gl_vertex_buffer* vertex_buffer; /** nullable */
  struct wl_listener vertex_buffer_destroy_listener;
};

static void z_render_block_state_vertex_buffer_destroy_signal_handler(struct wl_listener* listener,
                                                                      void* data)
{
  UNUSED(data);
  struct z_render_block_state* render_block_state;

  render_block_state = wl_container_of(listener, render_block_state, vertex_buffer_destroy_listener);

  render_block_state->vertex_buffer = NULL;
}

struct z_render_block_state* z_render_block_state_create()
{
  struct z_render_block_state* state;

  state = zalloc(sizeof *state);
  if (state == NULL) goto fail;

  wl_list_init(&state->vertex_buffer_destroy_listener.link);
  state->vertex_buffer_destroy_listener.notify = z_render_block_state_vertex_buffer_destroy_signal_handler;

  return state;

fail:
  return NULL;
}

void z_render_block_state_destroy(struct z_render_block_state* state)
{
  wl_list_remove(&state->vertex_buffer_destroy_listener.link);
  free(state);
}

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
