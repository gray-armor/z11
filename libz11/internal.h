#ifndef LIBZ11_INTERNAL_h
#define LIBZ11_INTERNAL_h

#ifdef __cplusplus
extern "C" {
#endif

#include <libz11.h>
#include <stdlib.h>

/* helper function */
inline void* zalloc(size_t size) { return calloc(1, size); }

/* z_compositor */

void z_compositor_append_render_block(struct z_compositor* compositor, struct z_render_block* render_block);

/* z_render_block */

struct z_render_block* z_render_block_create(struct wl_client* client, uint32_t id,
                                             struct z_compositor* compositor);

struct wl_list* z_render_block_get_link(struct z_render_block* render_block);

#ifdef __cplusplus
}
#endif

#endif  // LIBZ11_INTERNAL_h
