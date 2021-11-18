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
  float model_matrix[16];
};

/* zazen_opengl_render_component_manager */
struct zazen_opengl_render_component_manager;

struct zazen_opengl_render_component_manager*
zazen_opengl_render_component_manager_create(struct wl_display* display);

struct wl_list*
zazen_opengl_render_component_manager_get_render_component_back_state_list(
    struct zazen_opengl_render_component_manager* manager);

/* zazen_opengl */
struct zazen_opengl;

struct zazen_opengl* zazen_opengl_create(struct wl_display* display);

/* zazen_cuboid_window */
struct zazen_cuboid_window;

/* zazen_seat */
struct zazen_seat;

struct zazen_seat* zazen_seat_create(
    struct wl_display* display,
    struct zazen_opengl_render_component_manager* render_component_manager,
    struct zazen_compositor* compositor);

/* zazen_shell */
struct zazen_shell;

struct zazen_shell* zazen_shell_create(
    struct wl_display* display,
    struct zazen_opengl_render_component_manager* manager,
    struct zazen_compositor* compositor);

/* zazen_data_device_manager */
struct zazen_data_device_manager;

struct zazen_data_device_manager* zazen_data_device_manager_create(
    struct wl_display* display);

#ifdef __cplusplus
}
#endif

#endif  // LIBZAZEN_h
