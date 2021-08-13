#include "opengl_texture_2d.h"

#include "z11-opengl-server-protocol.h"
#include "z11-server-protocol.h"

static void zazen_opengl_texture_2d_destroy(struct zazen_opengl_texture_2d* texture);

static void zazen_opengl_texture_2d_handle_destroy(struct wl_resource* resource)
{
  struct zazen_opengl_texture_2d* texture = wl_resource_get_user_data(resource);

  zazen_opengl_texture_2d_destroy(texture);
}

static void zazen_opengl_texture_2d_protocol_destroy(struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void zazen_opengl_texture_2d_raw_buffer_destroy_listener(struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zazen_opengl_texture_2d* texture;
  struct zazen_opengl_texture_2d_state* state;

  texture = wl_container_of(listener, texture, raw_buffer_destroy_listener);

  wl_list_remove(&texture->raw_buffer_destroy_listener.link);
  wl_list_init(&texture->raw_buffer_destroy_listener.link);
  state = texture->state;
  texture->state = NULL;
  free(state);
}

static void zazen_opengl_texture_2d_protocol_set_image(struct wl_client* client, struct wl_resource* resource,
                                                       struct wl_resource* raw_buffer,
                                                       enum z11_opengl_texture_2d_format format,
                                                       int32_t width, int32_t height)
{
  UNUSED(client);
  struct zazen_opengl_texture_2d* texture;

  texture = wl_resource_get_user_data(resource);

  if (texture->state == NULL) {
    texture->state = zalloc(sizeof *texture->state);
    if (texture->state == NULL) {
      wl_client_post_no_memory(client);
      wl_resource_destroy(resource);
      return;
    }
  }

  wl_list_remove(&texture->raw_buffer_destroy_listener.link);
  wl_list_init(&texture->raw_buffer_destroy_listener.link);
  wl_resource_add_destroy_listener(raw_buffer, &texture->raw_buffer_destroy_listener);

  texture->state->raw_buffer_resource = raw_buffer;
  texture->state->format = format;
  texture->state->width = width;
  texture->state->height = height;

  wl_signal_emit(&texture->state_change_signal, texture);
}

static const struct z11_opengl_texture_2d_interface zazen_opengl_texture_2d_interface = {
    .destroy = zazen_opengl_texture_2d_protocol_destroy,
    .set_image = zazen_opengl_texture_2d_protocol_set_image,
};

struct zazen_opengl_texture_2d* zazen_opengl_texture_2d_create(struct wl_client* client, uint32_t id)
{
  struct zazen_opengl_texture_2d* texture;
  struct wl_resource* resource;

  texture = zalloc(sizeof *texture);
  if (texture == NULL) goto no_mem_texture;

  resource = wl_resource_create(client, &z11_opengl_texture_2d_interface, 1, id);
  if (resource == NULL) goto no_mem_resource;

  wl_resource_set_implementation(resource, &zazen_opengl_texture_2d_interface, texture,
                                 zazen_opengl_texture_2d_handle_destroy);

  wl_signal_init(&texture->destroy_signal);
  wl_signal_init(&texture->state_change_signal);

  texture->state = NULL;
  texture->raw_buffer_destroy_listener.notify = zazen_opengl_texture_2d_raw_buffer_destroy_listener;
  wl_list_init(&texture->raw_buffer_destroy_listener.link);

  return texture;

no_mem_resource:
  free(texture);

no_mem_texture:
  wl_client_post_no_memory(client);
  return NULL;
}

static void zazen_opengl_texture_2d_destroy(struct zazen_opengl_texture_2d* texture)
{
  wl_signal_emit(&texture->destroy_signal, texture);
  wl_list_remove(&texture->raw_buffer_destroy_listener.link);
  if (texture->state != NULL) free(texture->state);
  free(texture);
}
