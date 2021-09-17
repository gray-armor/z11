#ifndef LIBZAZEN_OPENGL_RENDER_ITEM_H
#define LIBZAZEN_OPENGL_RENDER_ITEM_H

#include <wayland-server.h>
#include <z11-opengl-server-protocol.h>

#include "opengl_render_component_back_state.h"
#include "opengl_render_component_manager.h"

struct zazen_opengl_render_item;

struct zazen_opengl_render_item* zazen_opengl_render_item_create(
    struct zazen_opengl_render_component_manager* manager);

void zazen_opengl_render_item_destroy(
    struct zazen_opengl_render_item* render_item);

// vertex_buffer_data must be alive until the next commit
void zazen_opengl_render_item_set_vertex_buffer(
    struct zazen_opengl_render_item* render_item, void* vertex_buffer_data,
    int32_t vertex_buffer_size, uint32_t vertex_buffer_stride);

void zazen_opengl_render_item_unset_vertex_buffer(
    struct zazen_opengl_render_item* render_item);

// texture_data must be alive until the next commit
void zazen_opengl_render_item_set_texture_2d(
    struct zazen_opengl_render_item* render_item, void* texture_data,
    enum z11_opengl_texture_2d_format format, int32_t width, int32_t height,
    int32_t buffer_size);

void zazen_opengl_render_item_unset_texture_2d(
    struct zazen_opengl_render_item* render_item);

// shader sources must be alive until the next commit
void zazen_opengl_render_item_set_shader(
    struct zazen_opengl_render_item* render_item,
    const char* vertex_shader_source, const char* fragment_shader_source);

void zazen_opengl_render_item_unset_shader(
    struct zazen_opengl_render_item* render_item);

void zazen_opengl_render_item_append_vertex_input_attribute(
    struct zazen_opengl_render_item* render_item, uint32_t location,
    enum z11_opengl_vertex_input_attribute_format format, uint32_t offset);

void zazen_opengl_render_item_clear_vertex_input_attribute(
    struct zazen_opengl_render_item* render_item);

void zazen_opengl_render_item_set_topology(
    struct zazen_opengl_render_item* render_item,
    enum z11_opengl_topology topology);

bool zazen_opengl_render_item_commit(
    struct zazen_opengl_render_item* render_item);

#endif  // LIBZAZEN_OPENGL_RENDER_ITEM_H
