#include "keyboard_client.h"

#include <wayland-server.h>
#include <z11-server-protocol.h>

#include "util.h"

static void zazen_keyboard_client_destroy(
    struct zazen_keyboard_client* keyboard_client);

static void zazen_keyboard_client_handle_destroy(struct wl_resource* resource)
{
  struct zazen_keyboard_client* keyboard_client;

  keyboard_client = wl_resource_get_user_data(resource);

  zazen_keyboard_client_destroy(keyboard_client);
}

static void zazen_keyboard_client_protocol_release(struct wl_client* client,
                                                   struct wl_resource* resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static const struct z11_keyboard_interface zazen_keyboard_interface = {
    .release = zazen_keyboard_client_protocol_release,
};

static void keyboard_destroy_signal_handler(struct wl_listener* listener,
                                            void* data)
{
  UNUSED(data);
  struct zazen_keyboard_client* keyboard_client;

  keyboard_client = wl_container_of(listener, keyboard_client,
                                    keyboard_destroy_signal_listener);

  wl_resource_destroy(keyboard_client->resource);
}

struct zazen_keyboard_client* zazen_keyboard_client_create(
    struct zazen_keyboard* keyboard, struct wl_client* client, uint32_t id)
{
  struct zazen_keyboard_client* keyboard_client;
  struct wl_resource* resource;

  keyboard_client = zalloc(sizeof *keyboard_client);
  if (keyboard_client == NULL) {
    wl_client_post_no_memory(client);
    return NULL;
  }

  resource = wl_resource_create(client, &z11_keyboard_interface, 1, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    goto out;
  }

  wl_resource_set_implementation(resource, &zazen_keyboard_interface,
                                 keyboard_client,
                                 zazen_keyboard_client_handle_destroy);

  keyboard_client->resource = resource;

  keyboard_client->keyboard_destroy_signal_listener.notify =
      keyboard_destroy_signal_handler;
  wl_signal_add(&keyboard->destroy_signal,
                &keyboard_client->keyboard_destroy_signal_listener);

  wl_list_insert(&keyboard->keyboard_clients, &keyboard_client->link);

  return keyboard_client;

out:
  free(keyboard_client);

  return NULL;
}

static void zazen_keyboard_client_destroy(
    struct zazen_keyboard_client* keyboard_client)
{
  wl_list_remove(&keyboard_client->keyboard_destroy_signal_listener.link);
  wl_list_remove(&keyboard_client->link);
  free(keyboard_client);
}
