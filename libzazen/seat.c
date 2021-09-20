#include "seat.h"

#include <string.h>

#include "ray_client.h"
#include "util.h"
#include "z11-input-server-protocol.h"

bool zazen_seat_init_keyboard(struct zazen_seat* seat)
{
  UNUSED(seat);
  // TODO: init keyboard
  return true;
}

bool zazen_seat_init_ray(struct zazen_seat* seat)
{
  struct zazen_ray* ray;

  // TODO: Handle capabilities

  if (seat->ray) {
    seat->ray_device_count += 1;
    return true;
  }

  ray = zazen_ray_create(seat);
  if (ray == NULL) {
    zazen_log("Unable to create ray\n");
    return false;
  }

  seat->ray = ray;
  seat->ray_device_count = 1;
  ray->seat = seat;

  return true;
}

static void zazen_seat_protocol_get_ray(struct wl_client* client,
                                        struct wl_resource* resource,
                                        uint32_t id)
{
  struct zazen_seat* seat;
  struct zazen_ray_client* ray_client;

  seat = wl_resource_get_user_data(resource);

  if (seat->ray == NULL) {
    zazen_log("The ray is unavailable");
    return;
  }

  // TODO: Handle the case ray client already created
  ray_client = zazen_ray_client_create(seat->ray, client, id);
  if (ray_client == NULL) {
    wl_client_post_no_memory(client);
    zazen_log("Failed to get a ray");
    return;
  }

  wl_list_insert(&seat->ray->ray_clients, &ray_client->link);
}

static const struct z11_seat_interface zazen_seat_interface = {
    .get_ray = zazen_seat_protocol_get_ray,
};

static void zazen_seat_bind(struct wl_client* client, void* data,
                            uint32_t version, uint32_t id)
{
  struct zazen_seat* seat = data;
  struct wl_resource* resource;

  resource = wl_resource_create(client, &z11_seat_interface, version, id);

  if (resource == NULL) {
    wl_client_post_no_memory(client);
    return;
  }

  wl_resource_set_implementation(resource, &zazen_seat_interface, seat, NULL);
}

struct zazen_seat* zazen_seat_create(
    struct wl_display* display,
    struct zazen_opengl_render_component_manager* render_component_manager)
{
  struct zazen_seat* seat;

  seat = zalloc(sizeof *seat);
  if (seat == NULL) return NULL;

  seat->render_component_manager = render_component_manager;

  seat->ray = NULL;
  seat->keyboard = NULL;

  seat->ray_device_count = 0;
  seat->keyboard_device_count = 0;

  seat->seat_name = "seat0";

  if (wl_global_create(display, &z11_seat_interface, 1, seat,
                       zazen_seat_bind) == NULL)
    goto out;

  seat->libinput = zazen_libinput_create(seat, display);
  if (seat->libinput == NULL) goto out;

  return seat;

out:
  free(seat);

  return NULL;
}

void zazen_seat_destroy(struct zazen_seat* seat)
{
  if (seat->ray) zazen_ray_destroy(seat->ray);
  if (seat->keyboard) zazen_keyboard_destroy(seat->keyboard);
  if (seat->libinput) zazen_libinput_destroy(seat->libinput);
  free(seat);
}
