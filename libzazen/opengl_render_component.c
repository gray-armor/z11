#include "opengl_render_component.h"

#include <stdio.h>
#include <wayland-server.h>
#include <wl_zext_server.h>

#include "opengl_render_component_back_state.h"
#include "opengl_render_component_manager.h"
#include "opengl_render_item.h"
#include "opengl_shader_program.h"
#include "opengl_texture_2d.h"
#include "opengl_vertex_buffer.h"
#include "util.h"
#include "virtual_object.h"
#include "z11-opengl-server-protocol.h"
#include "z11-server-protocol.h"

static void zazen_opengl_render_component_destroy(
    struct zazen_opengl_render_component* render_component);

static void zazen_opengl_render_component_handle_destroy(
    struct wl_resource* resource)
{
  struct zazen_opengl_render_component* render_component =
      wl_resource_get_user_data(resource);

  zazen_opengl_render_component_destroy(render_component);
}

static void zazen_opengl_render_component_protocol_destroy(
    struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void vertex_buffer_state_change_listener(struct wl_listener* listener,
                                                void* data)
{
  UNUSED(data);
  struct zazen_opengl_render_component* render_component;

  render_component = wl_container_of(listener, render_component,
                                     vertex_buffer_state_change_listener);

  if (render_component->vertex_buffer == NULL ||
      render_component->vertex_buffer->raw_buffer_resource == NULL) {
  } else {
    struct wl_zext_shm_raw_buffer* shm_raw_buffer;
    void* data;
    int32_t buffer_size;

    shm_raw_buffer = wl_zext_shm_raw_buffer_get(
        render_component->vertex_buffer->raw_buffer_resource);
    data = wl_zext_shm_raw_buffer_get_data(shm_raw_buffer);
    buffer_size = wl_zext_shm_raw_buffer_get_size(shm_raw_buffer);
    zazen_opengl_render_item_set_vertex_buffer(
        render_component->render_item, data, buffer_size,
        render_component->vertex_buffer->stride);
  }
}

static void vertex_buffer_destroy_listener(struct wl_listener* listener,
                                           void* data)
{
  UNUSED(data);
  struct zazen_opengl_render_component* render_component;

  render_component = wl_container_of(listener, render_component,
                                     vertex_buffer_destroy_listener);

  wl_list_remove(&render_component->vertex_buffer_destroy_listener.link);
  wl_list_init(&render_component->vertex_buffer_destroy_listener.link);
  wl_list_remove(&render_component->vertex_buffer_state_change_listener.link);
  wl_list_init(&render_component->vertex_buffer_state_change_listener.link);

  render_component->vertex_buffer = NULL;

  zazen_opengl_render_item_unset_vertex_buffer(render_component->render_item);
}

static void zazen_opengl_render_component_protocol_attach_vertex_buffer(
    struct wl_client* client, struct wl_resource* resource,
    struct wl_resource* vertex_buffer_resource)
{
  UNUSED(client);
  struct zazen_opengl_render_component* render_component;
  struct zazen_opengl_vertex_buffer* vertex_buffer;

  render_component = wl_resource_get_user_data(resource);
  vertex_buffer = wl_resource_get_user_data(vertex_buffer_resource);

  wl_list_remove(&render_component->vertex_buffer_state_change_listener.link);
  wl_signal_add(&vertex_buffer->state_change_signal,
                &render_component->vertex_buffer_state_change_listener);

  wl_list_remove(&render_component->vertex_buffer_destroy_listener.link);
  wl_signal_add(&vertex_buffer->destroy_signal,
                &render_component->vertex_buffer_destroy_listener);

  render_component->vertex_buffer = vertex_buffer;

  if (render_component->vertex_buffer->raw_buffer_resource == NULL) {
    zazen_opengl_render_item_unset_vertex_buffer(render_component->render_item);
  } else {
    struct wl_zext_shm_raw_buffer* shm_raw_buffer;
    void* data;
    int32_t buffer_size;

    shm_raw_buffer = wl_zext_shm_raw_buffer_get(
        render_component->vertex_buffer->raw_buffer_resource);
    data = wl_zext_shm_raw_buffer_get_data(shm_raw_buffer);
    buffer_size = wl_zext_shm_raw_buffer_get_size(shm_raw_buffer);
    zazen_opengl_render_item_set_vertex_buffer(
        render_component->render_item, data, buffer_size,
        render_component->vertex_buffer->stride);
  }
}

static void shader_program_state_change_listener(struct wl_listener* listener,
                                                 void* data)
{
  UNUSED(data);
  struct zazen_opengl_render_component* render_component;

  render_component = wl_container_of(listener, render_component,
                                     shader_program_state_change_listener);

  if (render_component->shader_program == NULL) {
    zazen_opengl_render_item_unset_shader(render_component->render_item);
  } else {
    zazen_opengl_render_item_set_shader(
        render_component->render_item,
        render_component->shader_program->vertex_shader_source,
        render_component->shader_program->fragment_shader_source);
  }
}

static void shader_program_destroy_handler(struct wl_listener* listener,
                                           void* data)
{
  UNUSED(data);
  struct zazen_opengl_render_component* render_component;

  render_component = wl_container_of(listener, render_component,
                                     shader_program_destroy_listener);

  wl_list_remove(&render_component->shader_program_destroy_listener.link);
  wl_list_init(&render_component->shader_program_destroy_listener.link);
  wl_list_remove(&render_component->shader_program_state_change_listener.link);
  wl_list_init(&render_component->shader_program_state_change_listener.link);

  render_component->shader_program = NULL;
  zazen_opengl_render_item_unset_shader(render_component->render_item);
}

static void zazen_opengl_render_component_protocol_attach_shader_program(
    struct wl_client* client, struct wl_resource* resource,
    struct wl_resource* shader_program_resource)
{
  UNUSED(client);
  struct zazen_opengl_render_component* render_component;
  struct zazen_opengl_shader_program* shader_program;

  render_component = wl_resource_get_user_data(resource);
  shader_program = wl_resource_get_user_data(shader_program_resource);

  wl_list_remove(&render_component->shader_program_state_change_listener.link);
  wl_signal_add(&shader_program->state_change_signal,
                &render_component->shader_program_state_change_listener);

  wl_list_remove(&render_component->shader_program_destroy_listener.link);
  wl_signal_add(&shader_program->destroy_signal,
                &render_component->shader_program_destroy_listener);

  render_component->shader_program = shader_program;
  zazen_opengl_render_item_set_shader(
      render_component->render_item,
      render_component->shader_program->vertex_shader_source,
      render_component->shader_program->fragment_shader_source);
}

static void texture_2d_state_change_listener(struct wl_listener* listener,
                                             void* data)
{
  UNUSED(data);
  struct zazen_opengl_render_component* render_component;

  render_component = wl_container_of(listener, render_component,
                                     texture_2d_state_change_listener);

  if (render_component->texture_2d == NULL ||
      render_component->texture_2d->state == NULL) {
    zazen_opengl_render_item_unset_texture_2d(render_component->render_item);
  } else {
    struct zazen_opengl_texture_2d_state* state =
        render_component->texture_2d->state;
    struct wl_zext_shm_raw_buffer* shm_raw_buffer;
    void* data;
    int32_t buffer_size;

    shm_raw_buffer = wl_zext_shm_raw_buffer_get(state->raw_buffer_resource);
    data = wl_zext_shm_raw_buffer_get_data(shm_raw_buffer);
    buffer_size = wl_zext_shm_raw_buffer_get_size(shm_raw_buffer);
    zazen_opengl_render_item_set_texture_2d(render_component->render_item, data,
                                            state->format, state->width,
                                            state->height, buffer_size);
  }
}

static void texture_2d_destroy_listener(struct wl_listener* listener,
                                        void* data)
{
  UNUSED(data);
  struct zazen_opengl_render_component* render_component;

  render_component =
      wl_container_of(listener, render_component, texture_2d_destroy_listener);

  wl_list_remove(&render_component->texture_2d_destroy_listener.link);
  wl_list_init(&render_component->texture_2d_destroy_listener.link);
  wl_list_remove(&render_component->texture_2d_state_change_listener.link);
  wl_list_init(&render_component->texture_2d_state_change_listener.link);

  render_component->texture_2d = NULL;
  zazen_opengl_render_item_unset_texture_2d(render_component->render_item);
}

static void zazen_opengl_render_component_protocol_attach_texture_2d(
    struct wl_client* client, struct wl_resource* resource,
    struct wl_resource* texture_2d_resource)
{
  UNUSED(client);
  struct zazen_opengl_render_component* render_component;
  struct zazen_opengl_texture_2d* texture_2d;

  render_component = wl_resource_get_user_data(resource);
  texture_2d = wl_resource_get_user_data(texture_2d_resource);

  wl_list_remove(&render_component->texture_2d_state_change_listener.link);
  wl_signal_add(&texture_2d->state_change_signal,
                &render_component->texture_2d_state_change_listener);

  wl_list_remove(&render_component->texture_2d_destroy_listener.link);
  wl_signal_add(&texture_2d->destroy_signal,
                &render_component->texture_2d_destroy_listener);

  render_component->texture_2d = texture_2d;

  if (render_component->texture_2d->state == NULL) {
    zazen_opengl_render_item_unset_texture_2d(render_component->render_item);
  } else {
    struct zazen_opengl_texture_2d_state* state =
        render_component->texture_2d->state;
    struct wl_zext_shm_raw_buffer* shm_raw_buffer;
    void* data;
    int32_t buffer_size;

    shm_raw_buffer = wl_zext_shm_raw_buffer_get(state->raw_buffer_resource);
    data = wl_zext_shm_raw_buffer_get_data(shm_raw_buffer);
    buffer_size = wl_zext_shm_raw_buffer_get_size(shm_raw_buffer);
    zazen_opengl_render_item_set_texture_2d(render_component->render_item, data,
                                            state->format, state->width,
                                            state->height, buffer_size);
  }
}

static void
zazen_opengl_render_component_protocol_append_vertex_input_attribute(
    struct wl_client* client, struct wl_resource* resource, uint32_t location,
    enum z11_opengl_vertex_input_attribute_format format, uint32_t offset)
{
  UNUSED(client);
  struct zazen_opengl_render_component* render_component;

  render_component = wl_resource_get_user_data(resource);

  zazen_opengl_render_item_append_vertex_input_attribute(
      render_component->render_item, location, format, offset);
}

static void
zazen_opengl_render_component_protocol_clear_vertex_input_attributes(
    struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);
  struct zazen_opengl_render_component* render_component;

  render_component = wl_resource_get_user_data(resource);

  zazen_opengl_render_item_clear_vertex_input_attribute(
      render_component->render_item);
}

static void zazen_opengl_render_component_protocol_set_topology(
    struct wl_client* client, struct wl_resource* resource,
    enum z11_opengl_topology topology)
{
  UNUSED(client);
  struct zazen_opengl_render_component* render_component;

  render_component = wl_resource_get_user_data(resource);

  zazen_opengl_render_item_set_topology(render_component->render_item,
                                        topology);
}

static const struct z11_opengl_render_component_interface
    zazen_opengl_render_component_interface = {
        .destroy = zazen_opengl_render_component_protocol_destroy,
        .attach_vertex_buffer =
            zazen_opengl_render_component_protocol_attach_vertex_buffer,
        .attach_shader_program =
            zazen_opengl_render_component_protocol_attach_shader_program,
        .attach_texture_2d =
            zazen_opengl_render_component_protocol_attach_texture_2d,
        .append_vertex_input_attribute =
            zazen_opengl_render_component_protocol_append_vertex_input_attribute,
        .clear_vertex_input_attributes =
            zazen_opengl_render_component_protocol_clear_vertex_input_attributes,
        .set_topology = zazen_opengl_render_component_protocol_set_topology,
};

static void virtual_object_destroy_signal_handler(struct wl_listener* listener,
                                                  void* data)
{
  struct zazen_virtual_object* virtual_object = data;
  struct zazen_opengl_render_component* render_component;

  wl_resource_post_error(
      virtual_object->resource, Z11_VIRTUAL_OBJECT_ERROR_DEFUNCT_COMPONENT,
      "Virtual object was destroyed before associated component");

  render_component = wl_container_of(listener, render_component,
                                     virtual_object_destroy_signal_listener);
  wl_resource_destroy(render_component->resource);
}

static void virtual_object_commit_signal_handler(struct wl_listener* listener,
                                                 void* data)
{
  UNUSED(data);
  struct zazen_opengl_render_component* render_component;

  render_component = wl_container_of(listener, render_component,
                                     virtual_object_commit_signal_listener);

  zazen_opengl_render_item_commit(render_component->render_item);
}

static void virtual_object_model_matrix_change_handler(
    struct wl_listener* listener, void* data)
{
  struct zazen_virtual_object* virtual_object = data;
  struct zazen_opengl_render_component* render_component;

  render_component = wl_container_of(
      listener, render_component, virtual_object_model_matrix_change_listener);

  zazen_opengl_render_item_set_model_matrix(render_component->render_item,
                                            virtual_object->model_matrix);
}

struct zazen_opengl_render_component* zazen_opengl_render_component_create(
    struct wl_client* client, uint32_t id,
    struct zazen_opengl_render_component_manager* manager,
    struct zazen_virtual_object* virtual_object)
{
  struct zazen_opengl_render_component* render_component;
  struct wl_resource* resource;

  render_component = zalloc(sizeof *render_component);
  if (render_component == NULL) {
    wl_client_post_no_memory(client);
    goto out;
  }

  render_component->render_item = zazen_opengl_render_item_create(manager);
  if (render_component->render_item == NULL) {
    wl_client_post_no_memory(client);
    goto out_component;
  }

  resource =
      wl_resource_create(client, &z11_opengl_render_component_interface, 1, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    goto out_render_item;
  }

  wl_resource_set_implementation(
      resource, &zazen_opengl_render_component_interface, render_component,
      zazen_opengl_render_component_handle_destroy);

  render_component->resource = resource;
  render_component->manager = manager;

  // Make sure that render_component->virtual_object will not be a dangling
  // pointer
  render_component->virtual_object_destroy_signal_listener.notify =
      virtual_object_destroy_signal_handler;
  wl_signal_add(&virtual_object->destroy_signal,
                &render_component->virtual_object_destroy_signal_listener);

  render_component->virtual_object_commit_signal_listener.notify =
      virtual_object_commit_signal_handler;
  wl_signal_add(&virtual_object->commit_signal,
                &render_component->virtual_object_commit_signal_listener);

  render_component->virtual_object_model_matrix_change_listener.notify =
      virtual_object_model_matrix_change_handler;
  wl_signal_add(&virtual_object->model_matrix_change_signal,
                &render_component->virtual_object_model_matrix_change_listener);

  render_component->vertex_buffer = NULL;
  render_component->vertex_buffer_state_change_listener.notify =
      vertex_buffer_state_change_listener;
  wl_list_init(&render_component->vertex_buffer_state_change_listener.link);
  render_component->vertex_buffer_destroy_listener.notify =
      vertex_buffer_destroy_listener;
  wl_list_init(&render_component->vertex_buffer_destroy_listener.link);

  render_component->shader_program = NULL;
  render_component->shader_program_state_change_listener.notify =
      shader_program_state_change_listener;
  wl_list_init(&render_component->shader_program_state_change_listener.link);
  render_component->shader_program_destroy_listener.notify =
      shader_program_destroy_handler;
  wl_list_init(&render_component->shader_program_destroy_listener.link);

  render_component->texture_2d = NULL;
  render_component->texture_2d_state_change_listener.notify =
      texture_2d_state_change_listener;
  wl_list_init(&render_component->texture_2d_state_change_listener.link);
  render_component->texture_2d_destroy_listener.notify =
      texture_2d_destroy_listener;
  wl_list_init(&render_component->texture_2d_destroy_listener.link);

  zazen_opengl_render_item_set_model_matrix(render_component->render_item,
                                            virtual_object->model_matrix);

  return render_component;

out_render_item:
  zazen_opengl_render_item_destroy(render_component->render_item);

out_component:
  free(render_component);

out:
  return NULL;
}

static void zazen_opengl_render_component_destroy(
    struct zazen_opengl_render_component* render_component)
{
  wl_list_remove(
      &render_component->virtual_object_destroy_signal_listener.link);
  wl_list_remove(&render_component->virtual_object_commit_signal_listener.link);
  wl_list_remove(&render_component->vertex_buffer_state_change_listener.link);
  wl_list_remove(&render_component->vertex_buffer_destroy_listener.link);
  wl_list_remove(&render_component->shader_program_state_change_listener.link);
  wl_list_remove(&render_component->shader_program_destroy_listener.link);
  wl_list_remove(&render_component->texture_2d_state_change_listener.link);
  wl_list_remove(&render_component->texture_2d_destroy_listener.link);
  zazen_opengl_render_item_destroy(render_component->render_item);
  free(render_component);
}
