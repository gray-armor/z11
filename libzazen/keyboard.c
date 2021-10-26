#include "keyboard.h"

#include <time.h>
#include <z11-server-protocol.h>

#include "cuboid_window.h"
#include "keyboard_client.h"
#include "keymap_info.h"
#include "seat.h"
#include "util.h"

void zazen_keyboard_notify_keymap(struct zazen_keyboard* keyboard,
                                  struct zazen_keyboard_client* keyboard_client)
{
  struct wl_resource* resource;
  struct zazen_keymap_info* info;

  info = keyboard->keymap_info;
  if (info == NULL) return;

  wl_resource_for_each(resource, &keyboard_client->keyboard_resources)
  {
    z11_keyboard_send_keymap(resource, info->format, info->fd, info->size);
  }
}

void zazen_keyboard_notify_key(struct zazen_keyboard* keyboard,
                               const struct timespec* time, uint32_t key,
                               enum wl_keyboard_key_state state)
{
  struct wl_resource* resource;
  struct zazen_keyboard_client* keyboard_client;
  struct zazen_cuboid_window* focus_cuboid_window;
  uint32_t serial, msecs;

  focus_cuboid_window = keyboard->focus_cuboid_window;
  if (focus_cuboid_window == NULL) return;

  serial = wl_display_next_serial(keyboard->seat->display);

  keyboard_client = zazen_keyboard_find_keyboard_client(
      keyboard, wl_resource_get_client(focus_cuboid_window->resource));
  if (keyboard_client == NULL) return;

  msecs = timespec_to_msec(time);

  wl_resource_for_each(resource, &keyboard_client->keyboard_resources)
  {
    z11_keyboard_send_key(resource, serial, msecs, key, state);
  }
}

static void zazen_keyboard_enter(struct zazen_keyboard* keyboard,
                                 struct zazen_cuboid_window* cuboid_window)
{
  struct zazen_keyboard_client* keyboard_client;
  struct wl_resource* keyboard_client_resource;
  uint32_t serial;

  keyboard_client = zazen_keyboard_find_keyboard_client(
      keyboard, wl_resource_get_client(cuboid_window->resource));
  if (keyboard_client == NULL) return;

  serial = wl_display_next_serial(keyboard->seat->display);

  wl_resource_for_each(keyboard_client_resource,
                       &keyboard_client->keyboard_resources)
  {
    z11_keyboard_send_enter(keyboard_client_resource, serial,
                            cuboid_window->resource, &keyboard->keys);
  }
}

static void zazen_keyboard_leave(struct zazen_keyboard* keyboard,
                                 struct zazen_cuboid_window* cuboid_window)
{
  struct zazen_keyboard_client* keyboard_client;
  struct wl_resource* keyboard_client_resource;
  uint32_t serial;

  keyboard_client = zazen_keyboard_find_keyboard_client(
      keyboard, wl_resource_get_client(cuboid_window->resource));
  if (keyboard_client == NULL) return;

  serial = wl_display_next_serial(keyboard->seat->display);

  wl_resource_for_each(keyboard_client_resource,
                       &keyboard_client->keyboard_resources)
  {
    z11_keyboard_send_leave(keyboard_client_resource, serial,
                            cuboid_window->resource);
  }
}

void zazen_keyboard_set_focus_cuboid_window(
    struct zazen_keyboard* keyboard, struct zazen_cuboid_window* cuboid_window)
{
  if (keyboard->focus_cuboid_window == cuboid_window) return;

  if (keyboard->focus_cuboid_window) {
    zazen_keyboard_leave(keyboard, keyboard->focus_cuboid_window);
    wl_list_remove(&keyboard->focus_cuboid_window_destroy_listener.link);
  }

  if (cuboid_window) {
    zazen_keyboard_enter(keyboard, cuboid_window);
    wl_signal_add(&cuboid_window->destroy_signal,
                  &keyboard->focus_cuboid_window_destroy_listener);
  }

  keyboard->focus_cuboid_window = cuboid_window;
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
  keyboard->focus_cuboid_window = NULL;

  keyboard->grab = NULL;

  wl_array_init(&keyboard->keys);

  keyboard->keymap_info = zazen_keymap_info_create();
  if (keyboard->keymap_info == NULL) goto out;

  wl_list_init(&keyboard->keyboard_clients);
  wl_signal_init(&keyboard->destroy_signal);

  keyboard->focus_cuboid_window_destroy_listener.notify =
      zazen_keyboard_focus_cuboid_window_destroy_handler;
  wl_list_init(&keyboard->focus_cuboid_window_destroy_listener.link);

  return keyboard;

out:
  free(keyboard);

  return NULL;
}

void zazen_keyboard_destroy(struct zazen_keyboard* keyboard)
{
  wl_array_release(&keyboard->keys);
  if (keyboard->keymap_info) zazen_keymap_info_destroy(keyboard->keymap_info);
  wl_signal_emit(&keyboard->destroy_signal, keyboard);
  wl_list_remove(&keyboard->focus_cuboid_window_destroy_listener.link);
  free(keyboard);
}
