#include "keyboard.h"

#include <z11-server-protocol.h>

#include "cuboid_window.h"
#include "keyboard_client.h"
#include "seat.h"
#include "util.h"

void zazen_keyboard_notify_key(struct zazen_keyboard* keyboard,
                               uint64_t time_usec, uint32_t key,
                               enum wl_keyboard_key_state state)
{
  struct wl_resource* resource;
  struct zazen_keyboard_client* keyboard_client;
  struct zazen_cuboid_window* focus_cuboid_window;
  uint32_t serial;

  focus_cuboid_window = keyboard->focus_cuboid_window;
  if (focus_cuboid_window == NULL) return;

  serial = wl_display_next_serial(keyboard->seat->display);

  keyboard_client = zazen_keyboard_find_keyboard_client(
      keyboard, wl_resource_get_client(focus_cuboid_window->resource));
  wl_resource_for_each(resource, &keyboard_client->keyboard_resources)
  {
    z11_keyboard_send_key(resource, serial, time_usec / 1000, key, state);
  }
}

static void zazen_keyboard_focus_cuboid_window_destroy_handler(
    struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zazen_keyboard* keyboard;
  keyboard =
      wl_container_of(listener, keyboard, focus_cuboid_window_destroy_listener);
  keyboard->focus_cuboid_window = NULL;
  wl_list_remove(&keyboard->focus_cuboid_window_destroy_listener.link);
}

struct zazen_keyboard_client* zazen_keyboard_find_keyboard_client(
    struct zazen_keyboard* keyboard, struct wl_client* client)
{
  struct zazen_keyboard_client* keyboard_client;

  wl_list_for_each(keyboard_client, &keyboard->keyboard_clients, link)
  {
    if (keyboard_client->client == client) return keyboard_client;
  }

  return NULL;
}

struct zazen_keyboard* zazen_keyboard_create(struct zazen_seat* seat)
{
  struct zazen_keyboard* keyboard;

  keyboard = zalloc(sizeof *keyboard);
  if (keyboard == NULL) return NULL;

  keyboard->seat = seat;

  keyboard->grab = NULL;

  wl_list_init(&keyboard->keyboard_clients);
  wl_signal_init(&keyboard->destroy_signal);

  keyboard->focus_cuboid_window_destroy_listener.notify =
      zazen_keyboard_focus_cuboid_window_destroy_handler;
  wl_list_init(&keyboard->focus_cuboid_window_destroy_listener.link);

  return keyboard;
}

void zazen_keyboard_destroy(struct zazen_keyboard* keyboard)
{
  wl_signal_emit(&keyboard->destroy_signal, keyboard);
  wl_list_remove(&keyboard->focus_cuboid_window_destroy_listener.link);
  free(keyboard);
}
