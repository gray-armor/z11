#include "seat.h"

#include <string.h>
#include <z11-input-server-protocol.h>

#include "util.h"

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
  struct zazen_ray* ray;

  seat = wl_resource_get_user_data(resource);

  ray = zazen_ray_create(seat);
  UNUSED(ray);
  UNUSED(client);
  UNUSED(id);
  if (seat == NULL) {
    zazen_log("Failed to create a ray");
  }
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
    struct zazen_opengl_render_component_manager* render_component_manager,
    const char* seat_name)
{
  struct zazen_seat* seat;

  seat = zalloc(sizeof *seat);
  if (seat == NULL) goto out;

  seat->render_component_manager = render_component_manager;

  seat->ray = NULL;
  seat->keyboard = NULL;

  seat->ray_device_count = 0;
  seat->keyboard_device_count = 0;

  seat->seat_name = strdup(seat_name);

  if (wl_global_create(display, &z11_seat_interface, 1, seat,
                       zazen_seat_bind) == NULL)
    goto out;

  return seat;

out:
  return NULL;
}

void zazen_seat_destroy(struct zazen_seat* seat)
{
  if (seat->ray) zazen_ray_destroy(seat->ray);
  if (seat->keyboard) zazen_keyboard_destroy(seat->keyboard);
  free(seat->seat_name);
  free(seat);
}
