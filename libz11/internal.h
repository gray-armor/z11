#ifndef LIBZ11_INTERNAL_h
#define LIBZ11_INTERNAL_h

#ifdef __cplusplus
extern "C" {
#endif

#include <GL/glew.h>
#include <libz11.h>
#include <stdlib.h>

#include "z11-server-protocol.h"

#define UNUSED(x) ((void)x)

/* helper function */
inline void* zalloc(size_t size) { return calloc(1, size); }

/* z_compositor */

void z_compositor_append_render_block(struct z_compositor* compositor, struct z_render_block* render_block);

/* z_gl_vertex_buffer */

struct z_gl_vertex_buffer {
  GLuint id;
  int32_t size;
  struct wl_signal destroy_signal;
};

struct z_gl_vertex_buffer* z_gl_vertex_buffer_create(struct wl_client* client, uint32_t id);

/* z_gl_shader_program */

struct z_gl_shader_program;

struct z_gl_shader_program* z_gl_shader_program_create(struct wl_client* client, uint32_t id,
                                                       const char* vertex_shader_source,
                                                       const char* fragment_shader_source);

void z_gl_shader_program_add_destroy_signal_handler(struct z_gl_shader_program* shader_program,
                                                    struct wl_listener* listener);

GLuint z_gl_shader_program_get_id(struct z_gl_shader_program* shader_program);

/* z_render_block_state */

struct z_render_block_state;

struct z_render_block_state* z_render_block_state_create();

void z_render_block_state_destroy(struct z_render_block_state* state);

void z_render_block_state_attach_vertex_buffer(struct z_render_block_state* state,
                                               struct z_gl_vertex_buffer* vertex_buffer);

struct z_gl_vertex_buffer* z_render_block_state_get_vertex_buffer(struct z_render_block_state* state);

void z_render_block_state_attach_shader_program(struct z_render_block_state* state,
                                                struct z_gl_shader_program* shader_program);

struct z_gl_shader_program* z_render_block_state_get_shader_program(struct z_render_block_state* state);

enum z11_gl_topology z_render_block_state_get_topology(struct z_render_block_state* state);

void z_render_block_state_set_topology(struct z_render_block_state* state, enum z11_gl_topology topology);

/* z_render_block */

struct z_render_block* z_render_block_create(struct wl_client* client, uint32_t id,
                                             struct z_compositor* compositor);

struct wl_list* z_render_block_get_link(struct z_render_block* render_block);

#ifdef __cplusplus
}
#endif

#endif  // LIBZ11_INTERNAL_h
