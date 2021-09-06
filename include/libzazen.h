#ifndef LIBZAZEN_h
#define LIBZAZEN_h

#ifdef __cplusplus
extern "C" {
#endif

#include <GL/glew.h>
#include <wayland-server.h>

/* zazen_compositor */
struct zazen_compositor;

struct zazen_compositor* zazen_compositor_create(struct wl_display* display);

void zazen_compositor_emit_frame_signal(struct zazen_compositor* compositor);

/* render component back state */

struct zazen_opengl_render_component_back_state {
  struct wl_list link;
  GLuint vertex_array_id;
  GLuint texture_2d_id;
  GLuint shader_program_id;
  GLuint vertex_buffer_id;
  int32_t vertex_buffer_size;
  uint32_t vertex_stride;
  GLenum topology_mode;
};

/* zazen_opengl_render_component_manager */
struct zazen_opengl_render_component_manager;

struct zazen_opengl_render_component_manager* zazen_opengl_render_component_manager_create(
    struct wl_display* display);

struct wl_list* zazen_opengl_render_component_manager_get_render_component_back_state_list(
    struct zazen_opengl_render_component_manager* manager);

// /* zazen_opengl */
struct zazen_opengl;

struct zazen_opengl* zazen_opengl_create(struct wl_display* display);

bool zazen_input_init(struct wl_event_loop* loop,
                      struct zazen_opengl_render_component_manager* render_component_manager);

void zazen_input_destroy();

#ifdef __cplusplus
}
#endif

#endif  // LIBZAZEN_h
