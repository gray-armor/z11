#include "opengl_render_component.h"

#include <stdio.h>
#include <wayland-server.h>

#include "opengl_render_component_back_state.h"
#include "opengl_render_component_manager.h"
#include "opengl_shader_program.h"
#include "opengl_texture_2d.h"
#include "opengl_vertex_buffer.h"
#include "util.h"
#include "virtual_object.h"
#include "z11-opengl-server-protocol.h"
#include "z11-server-protocol.h"

static void zazen_opengl_render_component_destroy(
    struct zazen_opengl_render_component* render_component);
static void zazen_opengl_render_component_commit(
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

  render_component->state_changed = true;
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

  render_component->state_changed = true;
}

static void shader_program_state_change_listener(struct wl_listener* listener,
                                                 void* data)
{
  UNUSED(data);
  struct zazen_opengl_render_component* render_component;

  render_component = wl_container_of(listener, render_component,
                                     shader_program_state_change_listener);

  render_component->state_changed = true;
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

  render_component->state_changed = true;
}

static void texture_2d_state_change_listener(struct wl_listener* listener,
                                             void* data)
{
  UNUSED(data);
  struct zazen_opengl_render_component* render_component;

  render_component = wl_container_of(listener, render_component,
                                     texture_2d_state_change_listener);

  render_component->state_changed = true;
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

  render_component->state_changed = true;
}

static void
zazen_opengl_render_component_protocol_append_vertex_input_attribute(
    struct wl_client* client, struct wl_resource* resource, uint32_t location,
    enum z11_opengl_vertex_input_attribute_format format, uint32_t offset)
{
  UNUSED(client);
  struct zazen_opengl_render_component* render_component;

  render_component = wl_resource_get_user_data(resource);

  struct zazen_opengl_render_component_back_state_vertex_input_attribute*
      input_attribute;

  input_attribute = wl_array_add(&render_component->vertex_input_attributes,
                                 sizeof *input_attribute);

  input_attribute->location = location;
  input_attribute->format = format;
  input_attribute->offset = offset;

  render_component->state_changed = true;
}

static void
zazen_opengl_render_component_protocol_clear_vertex_input_attributes(
    struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);
  struct zazen_opengl_render_component* render_component;

  render_component = wl_resource_get_user_data(resource);

  wl_array_release(&render_component->vertex_input_attributes);
  wl_array_init(&render_component->vertex_input_attributes);

  render_component->state_changed = true;
}

static void zazen_opengl_render_component_protocol_set_topology(
    struct wl_client* client, struct wl_resource* resource,
    enum z11_opengl_topology topology)
{
  UNUSED(client);
  struct zazen_opengl_render_component* render_component;

  render_component = wl_resource_get_user_data(resource);

  render_component->topology = topology;

  render_component->state_changed = true;
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

  if (render_component->state_changed) {
    zazen_opengl_render_component_commit(render_component);
    render_component->state_changed = false;
  }
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

  resource =
      wl_resource_create(client, &z11_opengl_render_component_interface, 1, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    goto out_component;
  }

  wl_resource_set_implementation(
      resource, &zazen_opengl_render_component_interface, render_component,
      zazen_opengl_render_component_handle_destroy);

  render_component->resource = resource;
  render_component->manager = manager;

  wl_list_init(&render_component->back_state.link);

  render_component->virtual_object_destroy_signal_listener.notify =
      virtual_object_destroy_signal_handler;
  // Make sure that render_component->virtual_object will not be a dangling
  // pointer
  wl_signal_add(&virtual_object->destroy_signal,
                &render_component->virtual_object_destroy_signal_listener);

  render_component->virtual_object_commit_signal_listener.notify =
      virtual_object_commit_signal_handler;
  wl_signal_add(&virtual_object->commit_signal,
                &render_component->virtual_object_commit_signal_listener);

  render_component->state_changed = true;

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

  wl_array_init(&render_component->vertex_input_attributes);

  render_component->topology = Z11_OPENGL_TOPOLOGY_LINES;

  return render_component;

out_component:
  free(render_component);

out:
  return NULL;
}

static void zazen_opengl_render_component_destroy(
    struct zazen_opengl_render_component* render_component)
{
  wl_array_release(&render_component->vertex_input_attributes);
  wl_list_remove(&render_component->back_state.link);
  wl_list_remove(
      &render_component->virtual_object_destroy_signal_listener.link);
  wl_list_remove(&render_component->virtual_object_commit_signal_listener.link);
  wl_list_remove(&render_component->vertex_buffer_state_change_listener.link);
  wl_list_remove(&render_component->vertex_buffer_destroy_listener.link);
  wl_list_remove(&render_component->shader_program_state_change_listener.link);
  wl_list_remove(&render_component->shader_program_destroy_listener.link);
  wl_list_remove(&render_component->texture_2d_state_change_listener.link);
  wl_list_remove(&render_component->texture_2d_destroy_listener.link);
  // TODO: clean up back state
  free(render_component);
}

static void commit_texture_2d(
    struct zazen_opengl_render_component* render_component);
static bool commit_shader_program(
    struct zazen_opengl_render_component* render_component);
static void commit_vertex_buffer(
    struct zazen_opengl_render_component* render_component);
static void commit_vertex_array(
    struct zazen_opengl_render_component* render_component);

static void zazen_opengl_render_component_commit(
    struct zazen_opengl_render_component* render_component)
{
  wl_list_remove(&render_component->back_state.link);
  wl_list_init(&render_component->back_state.link);

  commit_texture_2d(render_component);
  if (commit_shader_program(render_component) == false) {
    // TODO: Error handle
    return;
  }
  commit_vertex_buffer(render_component);

  zazen_opengl_render_component_back_state_set_topology_mode(
      &render_component->back_state, render_component->topology);

  commit_vertex_array(render_component);

  wl_list_insert(&render_component->manager->render_component_back_state_list,
                 &render_component->back_state.link);
}

static void commit_texture_2d(
    struct zazen_opengl_render_component* render_component)
{
  struct zazen_opengl_texture_2d_state* state;
  struct wl_shm_raw_buffer* shm_raw_buffer;
  int32_t buffer_size;
  void* data;

  zazen_opengl_render_component_back_state_delete_texture_2d(
      &render_component->back_state);

  if (render_component->texture_2d == NULL ||
      render_component->texture_2d->state == NULL)
    return;

  state = render_component->texture_2d->state;

  shm_raw_buffer = wl_shm_raw_buffer_get(state->raw_buffer_resource);
  buffer_size = wl_shm_raw_buffer_get_size(shm_raw_buffer);
  data = wl_shm_raw_buffer_get_data(shm_raw_buffer);

  zazen_opengl_render_component_back_state_generate_texture_2d(
      &render_component->back_state, state->format, state->width, state->height,
      data, buffer_size);
}

static bool commit_shader_program(
    struct zazen_opengl_render_component* render_component)
{
  zazen_opengl_render_component_back_state_delete_shader_program(
      &render_component->back_state);

  if (render_component->shader_program == NULL) return true;

  return zazen_opengl_render_component_back_state_generate_shader_program(
      &render_component->back_state,
      render_component->shader_program->vertex_shader_source,
      render_component->shader_program->fragment_shader_source);
}

static void commit_vertex_buffer(
    struct zazen_opengl_render_component* render_component)
{
  struct wl_shm_raw_buffer* shm_raw_buffer;
  void* data;
  int32_t buffer_size;

  zazen_opengl_render_component_back_state_delete_vertex_buffer(
      &render_component->back_state);

  if (render_component->vertex_buffer->raw_buffer_resource == NULL) return;

  shm_raw_buffer = wl_shm_raw_buffer_get(
      render_component->vertex_buffer->raw_buffer_resource);
  data = wl_shm_raw_buffer_get_data(shm_raw_buffer);
  buffer_size = wl_shm_raw_buffer_get_size(shm_raw_buffer);

  zazen_opengl_render_component_back_state_generate_vertex_buffer(
      &render_component->back_state, buffer_size, data,
      render_component->vertex_buffer->stride);
}

static void commit_vertex_array(
    struct zazen_opengl_render_component* render_component)
{
  zazen_opengl_render_component_back_state_delete_vertex_array(
      &render_component->back_state);

  if (render_component->back_state.vertex_buffer_id == 0 ||
      render_component->back_state.shader_program_id == 0) {
    return;
  }

  zazen_opengl_render_component_back_state_generate_vertex_array(
      &render_component->back_state,
      &render_component->vertex_input_attributes);
}
