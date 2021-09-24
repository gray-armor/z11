#include "ray_client.h"

#include <wayland-server.h>

#include "util.h"
#include "z11-input-server-protocol.h"

static void zazen_ray_client_destroy(struct zazen_ray_client *ray_client);

static void zazen_ray_client_handle_destroy(struct wl_resource *resource)
{
  struct zazen_ray_client *ray_client;

  ray_client = wl_resource_get_user_data(resource);

  zazen_ray_client_destroy(ray_client);
}

static void zazen_ray_client_protocol_release(struct wl_client *client,
                                              struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static const struct z11_ray_interface zazen_ray_interface = {
    .release = zazen_ray_client_protocol_release,
};

static void ray_destroy_signal_handler(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zazen_ray_client *ray_client;

  ray_client =
      wl_container_of(listener, ray_client, ray_destroy_signal_listener);

  wl_resource_destroy(ray_client->resource);
}

struct zazen_ray_client *zazen_ray_client_create(struct zazen_ray *ray,
                                                 struct wl_client *client,
                                                 uint32_t id)
{
  struct zazen_ray_client *ray_client;
  struct wl_resource *resource;

  ray_client = zalloc(sizeof *ray_client);
  if (ray_client == NULL) {
    wl_client_post_no_memory(client);
    return NULL;
  }

  resource = wl_resource_create(client, &z11_ray_interface, 1, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    goto out;
  }

  wl_resource_set_implementation(resource, &zazen_ray_interface, ray_client,
                                 zazen_ray_client_handle_destroy);

  ray_client->resource = resource;

  ray_client->ray_destroy_signal_listener.notify = ray_destroy_signal_handler;
  wl_signal_add(&ray->destroy_signal, &ray_client->ray_destroy_signal_listener);

  return ray_client;

out:
  free(ray_client);

  return NULL;
}

static void zazen_ray_client_destroy(struct zazen_ray_client *ray_client)
{
  wl_list_remove(&ray_client->ray_destroy_signal_listener.link);
  wl_list_remove(&ray_client->link);
  free(ray_client);
}
