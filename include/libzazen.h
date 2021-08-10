#ifndef LIBZAZEN_h
#define LIBZAZEN_h

#ifdef __cplusplus
extern "C" {
#endif

#include <wayland-server.h>

/* zazen_compositor */
struct zazen_compositor;

struct zazen_compositor* zazen_compositor_create(struct wl_display* display);

struct wl_list* zazen_compositor_get_render_element_list(struct zazen_compositor* compositor);

/* zazen_render_element */
struct zazen_render_element;

void zazen_render_element_draw(struct zazen_render_element* render_element,
                               const float* view_projection_matrix);

struct zazen_render_element* zazen_render_element_from_link(struct wl_list* link);

/* zazen_gl */
struct zazen_gl;

struct zazen_gl* zazen_gl_create(struct wl_display* display);

#ifdef __cplusplus
}
#endif

#endif  // LIBZAZEN_h
