#include "data_device.h"

#include <wayland-server.h>

#include "ray.h"
#include "seat.h"
#include "util.h"
#include "virtual_object.h"
#include "z11-server-protocol.h"

static void zazen_data_device_destroy(struct zazen_data_device* data_device);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
static void zazen_data_device_protocol_start_drag(
    struct wl_client* client, struct wl_resource* resource,
    struct wl_resource* source_resource, struct wl_resource* origin_resource,
    struct wl_resource* icon_resource, uint32_t serial)
{
  struct zazen_data_device* data_device = wl_resource_get_user_data(resource);
  struct zazen_seat* seat = data_device->seat;
  struct zazen_ray* ray = zazen_seat_get_ray(seat);
  struct zazen_data_source* data_source =
      wl_resource_get_user_data(source_resource);
  struct zazen_virtual_object* origin_virtual_object =
      wl_resource_get_user_data(origin_resource);
  struct zazen_virtual_object* icon_virtual_object =
      wl_resource_get_user_data(icon_resource);

  UNUSED(client);
  UNUSED(data_device);
  UNUSED(seat);
  UNUSED(ray);
  UNUSED(data_source);
  UNUSED(origin_virtual_object);
  UNUSED(icon_virtual_object);
}

static void zazen_data_device_protocol_release(struct wl_client* client,
                                               struct wl_resource* resource)
{}
#pragma GCC diagnostic pop

static void client_destroy_handler(struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zazen_data_device* data_device;

  data_device = wl_container_of(listener, data_device, client_destroy_listener);

  zazen_data_device_destroy(data_device);
}

static const struct z11_data_device_interface data_device_interface = {
    .start_drag = zazen_data_device_protocol_start_drag,
    .release = zazen_data_device_protocol_release,
};

void zazen_data_device_add_resource(struct zazen_data_device* data_device,
                                    struct wl_resource* resource)
{
  wl_resource_set_implementation(resource, &data_device_interface, data_device,
                                 NULL);

  wl_list_insert(&data_device->data_device_resources,
                 wl_resource_get_link(resource));
}

static struct zazen_data_device* zazen_data_device_create(
    struct zazen_seat* seat, struct wl_client* client)
{
  struct zazen_data_device* data_device;

  data_device = zalloc(sizeof *data_device);
  if (data_device == NULL) {
    wl_client_post_no_memory(client);
  }

  data_device->seat = seat;
  seat->data_device = data_device;

  data_device->client = client;

  wl_list_init(&data_device->data_device_resources);

  data_device->client_destroy_listener.notify = client_destroy_handler;
  wl_client_add_destroy_listener(data_device->client,
                                 &data_device->client_destroy_listener);

  return data_device;
}

struct zazen_data_device* zazen_data_device_find_or_create(
    struct zazen_seat* seat, struct wl_client* client)
{
  struct zazen_data_device* data_device;

  data_device = seat->data_device;
  if (data_device) return data_device;

  data_device = zazen_data_device_create(seat, client);
  if (data_device == NULL) {
    wl_client_post_no_memory(client);
    return NULL;
  }

  return data_device;
}

static void zazen_data_device_destroy(struct zazen_data_device* data_device)
{
  struct wl_resource* resource;

  wl_resource_for_each(resource, &data_device->data_device_resources)
  {
    wl_resource_set_user_data(resource, NULL);
  }

  wl_list_remove(&data_device->client_destroy_listener.link);

  data_device->seat->data_device = NULL;

  free(data_device);
}
