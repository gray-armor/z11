#include "ray_client.h"

#include <wayland-server.h>

#include "ray.h"
#include "util.h"
#include "z11-input-server-protocol.h"

static void zazen_ray_client_destroy(struct zazen_ray_client *ray_client);

static void zazen_ray_client_protocol_release(struct wl_client *client,
                                              struct wl_resource *resource)
{
  UNUSED(client);
  wl_list_remove(wl_resource_get_link(resource));
  wl_resource_destroy(resource);
}

static const struct z11_ray_interface zazen_ray_interface = {
    .release = zazen_ray_client_protocol_release,
};

static void client_destroy_handler(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zazen_ray_client *ray_client;

  ray_client = wl_container_of(listener, ray_client, client_destroy_listener);

  zazen_ray_client_destroy(ray_client);
}

static void ray_destroy_signal_handler(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zazen_ray_client *ray_client;

  ray_client =
      wl_container_of(listener, ray_client, ray_destroy_signal_listener);

  zazen_ray_client_destroy(ray_client);
}

void zazen_ray_client_add_resource(struct zazen_ray_client *ray_client,
                                   struct wl_resource *resource)
{
  wl_resource_set_implementation(resource, &zazen_ray_interface, ray_client,
                                 NULL);

  wl_list_insert(&ray_client->ray_resources, wl_resource_get_link(resource));
}

struct zazen_ray_client *zazen_ray_client_create(struct zazen_ray *ray,
                                                 struct wl_client *client)
{
  struct zazen_ray_client *ray_client;

  ray_client = zalloc(sizeof *ray_client);
  if (ray_client == NULL) {
    wl_client_post_no_memory(client);
    return NULL;
  }

  ray_client->client = client;

  wl_list_init(&ray_client->ray_resources);

  ray_client->client_destroy_listener.notify = client_destroy_handler;
  wl_client_add_destroy_listener(ray_client->client,
                                 &ray_client->client_destroy_listener);

  ray_client->ray_destroy_signal_listener.notify = ray_destroy_signal_handler;
  wl_signal_add(&ray->destroy_signal, &ray_client->ray_destroy_signal_listener);

  wl_list_insert(&ray->ray_clients, &ray_client->link);

  return ray_client;
}

struct zazen_ray_client *zazen_ray_client_find_or_create(
    struct zazen_ray *ray, struct wl_client *client)
{
  struct zazen_ray_client *ray_client;

  ray_client = zazen_ray_find_ray_client(ray, client);
  if (ray_client) return ray_client;

  ray_client = zazen_ray_client_create(ray, client);
  if (ray_client == NULL) {
    wl_client_post_no_memory(client);
    return NULL;
  }

  return ray_client;
}

static void zazen_ray_client_destroy(struct zazen_ray_client *ray_client)
{
  struct wl_resource *resource;

  wl_resource_for_each(resource, &ray_client->ray_resources)
  {
    wl_resource_set_user_data(resource, NULL);
  }

  wl_list_remove(&ray_client->client_destroy_listener.link);
  wl_list_remove(&ray_client->ray_destroy_signal_listener.link);

  wl_list_remove(&ray_client->link);

  free(ray_client);
}
