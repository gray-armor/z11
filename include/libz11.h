#ifndef LIBZ11_h
#define LIBZ11_h

#ifdef __cplusplus
extern "C" {
#endif

#include <wayland-server.h>

/* z_compositor */
struct z_compositor;

struct z_compositor* z_compositor_create(struct wl_display* display);

struct wl_list* z_compositor_get_render_block_list(struct z_compositor* compositor);

/* z11_render_block */
struct z_render_block;

void z_render_block_draw(struct z_render_block* render_block, const float* view_projection_matrix);

struct z_render_block* z_render_block_from_link(struct wl_list* link);

/* z_gl */
struct z_gl;

struct z_gl* z_gl_create(struct wl_display* display);

#ifdef __cplusplus
}
#endif

#endif  // LIBZ11_h
