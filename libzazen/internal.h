#ifndef LIBZAZEN_INTERNAL_h
#define LIBZAZEN_INTERNAL_h

#ifdef __cplusplus
extern "C" {
#endif

#include <GL/glew.h>
#include <libzazen.h>
#include <stdlib.h>

#include "z11-server-protocol.h"

#define UNUSED(x) ((void)x)

/* helper function */
inline void* zalloc(size_t size) { return calloc(1, size); }

/* util */
struct zazen_week_ref;

typedef void (*zazen_week_ref_destroy_func_t)(struct zazen_week_ref* ref);

struct zazen_week_ref {
  void* data;  // NULLABLE
  struct wl_listener destroy_listener;
  zazen_week_ref_destroy_func_t destroy_func;
};

void zazen_week_ref_init(struct zazen_week_ref* week_ref);

void zazen_week_ref_destroy(struct zazen_week_ref* ref);

void zazen_week_ref_set_data(struct zazen_week_ref* ref, void* data, struct wl_signal* destroy_signal,
                             zazen_week_ref_destroy_func_t on_destroy);

/* zazen_compositor */

void zazen_compositor_append_render_element(struct zazen_compositor* compositor,
                                            struct zazen_render_element* render_element);

/* zazen_gl_vertex_buffer */

struct zazen_gl_vertex_buffer {
  GLuint id;
  int32_t size;
  struct wl_signal destroy_signal;
};

struct zazen_gl_vertex_buffer* zazen_gl_vertex_buffer_create(struct wl_client* client, uint32_t id);

/* zazen_gl_texture_2d */

struct zazen_gl_texture_2d;

struct zazen_gl_texture_2d* zazen_gl_texture_2d_create(struct wl_client* client, uint32_t id);

struct wl_signal* zazen_gl_texture_2d_get_destroy_signal(struct zazen_gl_texture_2d* texture);

GLuint zazen_gl_texture_2d_get_id(struct zazen_gl_texture_2d* texture);

/* zazen_gl_shader_program */

struct zazen_gl_shader_program;

struct zazen_gl_shader_program* zazen_gl_shader_program_create(struct wl_client* client, uint32_t id,
                                                               const char* vertex_shader_source,
                                                               const char* fragment_shader_source);

struct wl_signal* zazen_gl_shader_program_get_destroy_signal(struct zazen_gl_shader_program* shader_program);

GLuint zazen_gl_shader_program_get_id(struct zazen_gl_shader_program* shader_program);

/* zazen_render_element_state */
struct zazen_gl_vertex_input_attribute;

struct zazen_render_element_state;

struct zazen_render_element_state* zazen_render_element_state_create();

void zazen_render_element_state_destroy(struct zazen_render_element_state* state);

void zazen_render_element_state_attach_vertex_buffer(struct zazen_render_element_state* state,
                                                     struct zazen_gl_vertex_buffer* vertex_buffer,
                                                     uint32_t vertex_stride);

/**
 * @return nullable
 */
struct zazen_gl_vertex_buffer* zazen_render_element_state_get_vertex_buffer(
    struct zazen_render_element_state* state);

uint32_t zazen_render_element_state_get_vertex_stride(struct zazen_render_element_state* state);

void zazen_render_element_state_attach_shader_program(struct zazen_render_element_state* state,
                                                      struct zazen_gl_shader_program* shader_program);

/**
 * @return nullable
 */
struct zazen_gl_shader_program* zazen_render_element_state_get_shader_program(
    struct zazen_render_element_state* state);

void zazen_render_element_state_attach_texture_2d(struct zazen_render_element_state* state,
                                                  struct zazen_gl_texture_2d* texture_2d);

/**
 * @return nullable
 */
struct zazen_gl_texture_2d* zazen_render_element_state_get_texture_2d(
    struct zazen_render_element_state* state);

void zazen_render_element_state_append_vertex_input_attribute(
    struct zazen_render_element_state* state, uint32_t location,
    enum z11_gl_vertex_input_attribute_format format, uint32_t offset);

struct wl_array* zazen_render_element_state_get_vertex_input_attributes(
    struct zazen_render_element_state* state);

enum z11_gl_topology zazen_render_element_state_get_topology(struct zazen_render_element_state* state);

void zazen_render_element_state_set_topology(struct zazen_render_element_state* state,
                                             enum z11_gl_topology topology);

struct zazen_gl_vertex_input_attribute {
  uint32_t location;
  enum z11_gl_vertex_input_attribute_format format;
  uint32_t offset;
};

/* zazen_render_element */

struct zazen_render_element* zazen_render_element_create(struct wl_client* client, uint32_t id,
                                                         struct zazen_compositor* compositor);

struct wl_list* zazen_render_element_get_link(struct zazen_render_element* render_element);

#ifdef __cplusplus
}
#endif

#endif  // LIBZAZEN_INTERNAL_h
