#include "data_device_manager.h"

#include <libzazen.h>
#include <wayland-server.h>

#include "data_device.h"
#include "data_source.h"
#include "util.h"
#include "z11-server-protocol.h"

static void zazen_data_device_manager_protocol_create_data_source(
    struct wl_client* client, struct wl_resource* resource, uint32_t id)
{
  struct wl_resource* data_source_resource;
  struct zazen_data_source* data_source;

  data_source_resource =
      wl_resource_create(client, &z11_data_source_interface,
                         wl_resource_get_version(resource), id);
  if (data_source_resource == NULL) {
    wl_resource_post_no_memory(resource);
    return;
  }

  data_source = zazen_data_source_create(client, data_source_resource, id);
  if (data_source == NULL) {
    wl_resource_post_no_memory(resource);
    zazen_log("Failed to create a data source\n");
    return;
  }
}

static void zazen_data_device_manager_protocol_get_data_device(
    struct wl_client* client, struct wl_resource* manager_resource, uint32_t id,
    struct wl_resource* seat_resource)
{
  struct wl_resource* data_device_resource;
  struct zazen_data_device* data_device;
  struct zazen_seat* seat = wl_resource_get_user_data(seat_resource);

  data_device_resource =
      wl_resource_create(client, &z11_data_device_interface,
                         wl_resource_get_version(manager_resource), id);
  if (data_device_resource == NULL) {
    wl_resource_post_no_memory(manager_resource);
    return;
  }

  data_device = zazen_data_device_find_or_create(seat, client);
  if (data_device == NULL) {
    wl_resource_post_no_memory(manager_resource);
    zazen_log("Failed to get a data device\n");
    return;
  }

  zazen_data_device_add_resource(data_device, data_device_resource);
}

static const struct z11_data_device_manager_interface
    zazen_data_device_manager_interface = {
        .create_data_source =
            zazen_data_device_manager_protocol_create_data_source,
        .get_data_device = zazen_data_device_manager_protocol_get_data_device,
};

static void zazen_data_device_manager_bind(struct wl_client* client, void* data,
                                           uint32_t version, uint32_t id)
{
  struct zazen_data_device_manager* data_device_manager = data;
  struct wl_resource* resource;

  resource = wl_resource_create(client, &z11_data_device_manager_interface,
                                version, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    return;
  }

  wl_resource_set_implementation(resource, &zazen_data_device_manager_interface,
                                 data_device_manager, NULL);
}

struct zazen_data_device_manager* zazen_data_device_manager_create(
    struct wl_display* display)
{
  struct zazen_data_device_manager* data_device_manager;

  data_device_manager = zalloc(sizeof *data_device_manager);
  if (data_device_manager == NULL) return NULL;

  if (wl_global_create(display, &z11_data_device_manager_interface, 1,
                       data_device_manager,
                       zazen_data_device_manager_bind) == NULL) {
    free(data_device_manager);
    return NULL;
  }

  return data_device_manager;
}
