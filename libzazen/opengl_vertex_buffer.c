#include "opengl_vertex_buffer.h"

#include "z11-opengl-server-protocol.h"
#include "z11-server-protocol.h"

static void zazen_opengl_vertex_buffer_destroy(struct zazen_opengl_vertex_buffer *vertex_buffer);

static void zazen_opengl_vertex_buffer_handle_destroy(struct wl_resource *resource)
{
  struct zazen_opengl_vertex_buffer *vertex_buffer = wl_resource_get_user_data(resource);

  zazen_opengl_vertex_buffer_destroy(vertex_buffer);
}

static void zazen_opengl_vertex_buffer_protocol_destroy(struct wl_client *client,
                                                        struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void zazen_opengl_vertex_buffer_raw_buffer_destroy_listener(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zazen_opengl_vertex_buffer *vertex_buffer;

  vertex_buffer = wl_container_of(listener, vertex_buffer, raw_buffer_resource_destroy_listener);

  wl_list_remove(&vertex_buffer->raw_buffer_resource_destroy_listener.link);
  wl_list_init(&vertex_buffer->raw_buffer_resource_destroy_listener.link);
  vertex_buffer->raw_buffer_resource = NULL;
  vertex_buffer->stride = 0;
}

static void zazen_opengl_vertex_buffer_protocol_attach(struct wl_client *client, struct wl_resource *resource,
                                                       _Nullable struct wl_resource *raw_buffer,
                                                       uint32_t vertex_stride)
{
  UNUSED(client);
  struct zazen_opengl_vertex_buffer *vertex_buffer = wl_resource_get_user_data(resource);

  vertex_buffer->raw_buffer_resource = raw_buffer;
  vertex_buffer->stride = vertex_stride;

  wl_list_remove(&vertex_buffer->raw_buffer_resource_destroy_listener.link);
  wl_resource_add_destroy_listener(raw_buffer, &vertex_buffer->raw_buffer_resource_destroy_listener);

  wl_signal_emit(&vertex_buffer->state_change_signal, vertex_buffer);
}

static const struct z11_opengl_vertex_buffer_interface zazen_opengl_vertex_buffer_interface = {
    .destroy = zazen_opengl_vertex_buffer_protocol_destroy,
    .attach = zazen_opengl_vertex_buffer_protocol_attach,
};

struct zazen_opengl_vertex_buffer *zazen_opengl_vertex_buffer_create(struct wl_client *client, uint32_t id)
{
  struct zazen_opengl_vertex_buffer *vertex_buffer;
  struct wl_resource *resource;

  vertex_buffer = zalloc(sizeof *vertex_buffer);
  if (vertex_buffer == NULL) goto fail;

  resource = wl_resource_create(client, &z11_opengl_vertex_buffer_interface, 1, id);
  if (resource == NULL) goto no_mem_resource;

  wl_signal_init(&vertex_buffer->destroy_signal);
  wl_signal_init(&vertex_buffer->state_change_signal);

  wl_resource_set_implementation(resource, &zazen_opengl_vertex_buffer_interface, vertex_buffer,
                                 zazen_opengl_vertex_buffer_handle_destroy);

  vertex_buffer->raw_buffer_resource = NULL;
  vertex_buffer->stride = 0;
  vertex_buffer->raw_buffer_resource_destroy_listener.notify =
      zazen_opengl_vertex_buffer_raw_buffer_destroy_listener;
  wl_list_init(&vertex_buffer->raw_buffer_resource_destroy_listener.link);

  return vertex_buffer;

no_mem_resource:
  free(vertex_buffer);

fail:
  wl_client_post_no_memory(client);
  return NULL;
}

static void zazen_opengl_vertex_buffer_destroy(struct zazen_opengl_vertex_buffer *vertex_buffer)
{
  wl_signal_emit(&vertex_buffer->destroy_signal, vertex_buffer);
  wl_list_remove(&vertex_buffer->raw_buffer_resource_destroy_listener.link);
  free(vertex_buffer);
}
