#include "seat.h"

#include <string.h>
#include <wayland-server.h>

#include "util.h"
#include "z11-input-server-protocol.h"

void zazen_seat_send_updated_capability(struct zazen_seat* seat)
{
  enum z11_seat_capability capability = 0;
  struct wl_resource* resource;

  if (seat->keyboard_device_count > 0)
    capability |= Z11_SEAT_CAPABILITY_KEYBOARD;
  if (seat->ray_device_count > 0) capability |= Z11_SEAT_CAPABILITY_RAY;

  wl_resource_for_each(resource, &seat->client_list)
  {
    z11_seat_send_capability(resource, capability);
  }
}

bool zazen_seat_init_keyboard(struct zazen_seat* seat)
{
  struct zazen_keyboard* keyboard;

  if (seat->keyboard) {
    seat->keyboard_device_count += 1;
    return true;
  }

  keyboard = zazen_keyboard_create(seat);
  if (keyboard == NULL) {
    zazen_log("Unable to create keyboard\n");
    return false;
  }

  seat->keyboard = keyboard;
  seat->keyboard_device_count = 1;
  keyboard->seat = seat;

  zazen_seat_send_updated_capability(seat);

  return true;
}

bool zazen_seat_init_ray(struct zazen_seat* seat)
{
  struct zazen_ray* ray;

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

  zazen_seat_send_updated_capability(seat);

  return true;
}

void zazen_seat_release_ray(struct zazen_seat* seat)
{
  seat->ray_device_count--;
  if (seat->ray_device_count == 0) {
    zazen_ray_destroy(seat->ray);
    seat->ray = NULL;

    zazen_seat_send_updated_capability(seat);
  }
}

void zazen_seat_release_keyboard(struct zazen_seat* seat)
{
  seat->keyboard_device_count--;
  if (seat->keyboard_device_count == 0) {
    zazen_keyboard_destroy(seat->keyboard);
    seat->keyboard = NULL;

    zazen_seat_send_updated_capability(seat);
  }
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
}

static void zazen_seat_protocol_get_keyboard(struct wl_client* client,
                                             struct wl_resource* resource,
                                             uint32_t id)
{
  struct zazen_seat* seat;
  struct zazen_keyboard_client* keyboard_client;

  seat = wl_resource_get_user_data(resource);

  if (seat->keyboard == NULL) {
    zazen_log("The keyboard is unavailable");
    return;
  }

  // TODO: Handle the case keyboard client already created
  keyboard_client = zazen_keyboard_client_create(seat->keyboard, client, id);
  if (keyboard_client == NULL) {
    wl_client_post_no_memory(client);
    zazen_log("Failed to get a keyboard");
    return;
  }
}

static const struct z11_seat_interface zazen_seat_interface = {
    .get_ray = zazen_seat_protocol_get_ray,
    .get_keyboard = zazen_seat_protocol_get_keyboard,
};

static void zazen_seat_unbind(struct wl_resource* resource)
{
  wl_list_remove(wl_resource_get_link(resource));
}

static void zazen_seat_bind(struct wl_client* client, void* data,
                            uint32_t version, uint32_t id)
{
  struct zazen_seat* seat = data;
  struct wl_resource* resource;
  enum z11_seat_capability capability = 0;

  resource = wl_resource_create(client, &z11_seat_interface, version, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    return;
  }

  wl_resource_set_implementation(resource, &zazen_seat_interface, seat,
                                 zazen_seat_unbind);

  wl_list_insert(&seat->client_list, wl_resource_get_link(resource));

  if (seat->keyboard_device_count > 0)
    capability |= Z11_SEAT_CAPABILITY_KEYBOARD;
  if (seat->ray_device_count > 0) capability |= Z11_SEAT_CAPABILITY_RAY;

  z11_seat_send_capability(resource, capability);
}

struct zazen_seat* zazen_seat_create(
    struct wl_display* display,
    struct zazen_opengl_render_component_manager* render_component_manager)
{
  struct zazen_seat* seat;

  seat = zalloc(sizeof *seat);
  if (seat == NULL) return NULL;

  seat->render_component_manager = render_component_manager;

  wl_list_init(&seat->client_list);

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
