#include "keyboard_client.h"

#include <wayland-server.h>
#include <z11-server-protocol.h>

#include "keyboard.h"
#include "util.h"

static void zazen_keyboard_client_destroy(
    struct zazen_keyboard_client* keyboard_client);

static void zazen_keyboard_client_protocol_release(struct wl_client* client,
                                                   struct wl_resource* resource)
{
  UNUSED(client);
  wl_list_remove(wl_resource_get_link(resource));
  wl_resource_destroy(resource);
}

static const struct z11_keyboard_interface zazen_keyboard_interface = {
    .release = zazen_keyboard_client_protocol_release,
};

static void client_destroy_handler(struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zazen_keyboard_client* keyboard_client;

  keyboard_client =
      wl_container_of(listener, keyboard_client, client_destroy_listener);

  zazen_keyboard_client_destroy(keyboard_client);
}

static void keyboard_destroy_signal_handler(struct wl_listener* listener,
                                            void* data)
{
  UNUSED(data);
  struct zazen_keyboard_client* keyboard_client;

  keyboard_client = wl_container_of(listener, keyboard_client,
                                    keyboard_destroy_signal_listener);

  zazen_keyboard_client_destroy(keyboard_client);
}

void zazen_keyboard_client_add_resource(
    struct zazen_keyboard_client* keyboard_client, struct wl_resource* resource)
{
  wl_resource_set_implementation(resource, &zazen_keyboard_interface,
                                 keyboard_client, NULL);

  wl_list_insert(&keyboard_client->keyboard_resources,
                 wl_resource_get_link(resource));
}

struct zazen_keyboard_client* zazen_keyboard_client_create(
    struct zazen_keyboard* keyboard, struct wl_client* client)
{
  struct zazen_keyboard_client* keyboard_client;

  keyboard_client = zalloc(sizeof *keyboard_client);
  if (keyboard_client == NULL) {
    wl_client_post_no_memory(client);
    return NULL;
  }

  keyboard_client->client = client;

  wl_list_init(&keyboard_client->keyboard_resources);

  keyboard_client->client_destroy_listener.notify = client_destroy_handler;
  wl_client_add_destroy_listener(keyboard_client->client,
                                 &keyboard_client->client_destroy_listener);

  keyboard_client->keyboard_destroy_signal_listener.notify =
      keyboard_destroy_signal_handler;
  wl_signal_add(&keyboard->destroy_signal,
                &keyboard_client->keyboard_destroy_signal_listener);

  wl_list_insert(&keyboard->keyboard_clients, &keyboard_client->link);

  return keyboard_client;
}

struct zazen_keyboard_client* zazen_keyboard_client_find_or_create(
    struct zazen_keyboard* keyboard, struct wl_client* client)
{
  struct zazen_keyboard_client* keyboard_client;

  keyboard_client = zazen_keyboard_find_keyboard_client(keyboard, client);
  if (keyboard_client) return keyboard_client;

  keyboard_client = zazen_keyboard_client_create(keyboard, client);
  if (keyboard_client == NULL) {
    wl_client_post_no_memory(client);
    return NULL;
  }

  return keyboard_client;
}

static void zazen_keyboard_client_destroy(
    struct zazen_keyboard_client* keyboard_client)
{
  struct wl_resource* resource;

  wl_resource_for_each(resource, &keyboard_client->keyboard_resources)
  {
    wl_resource_set_user_data(resource, NULL);
  }

  wl_list_remove(&keyboard_client->client_destroy_listener.link);
  wl_list_remove(&keyboard_client->keyboard_destroy_signal_listener.link);

  wl_list_remove(&keyboard_client->link);

  free(keyboard_client);
}
