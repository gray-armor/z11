#include "shell.h"

#include <wayland-server.h>
#include <z11-server-protocol.h>

#include "cuboid_window.h"
#include "util.h"

static void zazen_shell_protocol_get_cuboid_window(
    struct wl_client* client, struct wl_resource* resource, uint32_t id,
    struct wl_resource* virtual_object)
{
  UNUSED(resource);
  UNUSED(virtual_object);
  struct zazen_cuboid_window* cuboid_window;

  cuboid_window = zazen_cuboid_window_create(client, id);
  if (cuboid_window == NULL) {
    zazen_log("Failed to create cuboid window");
  }
}

static const struct z11_shell_interface zazen_shell_interface = {
    .get_cuboid_window = zazen_shell_protocol_get_cuboid_window,
};

static void zazen_shell_bind(struct wl_client* client, void* data,
                             uint32_t version, uint32_t id)
{
  struct zazen_shell* shell = data;
  struct wl_resource* resource;

  resource = wl_resource_create(client, &z11_shell_interface, version, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    return;
  }

  wl_resource_set_implementation(resource, &zazen_shell_interface, shell, NULL);
}

struct zazen_shell* zazen_shell_create(struct wl_display* display)
{
  struct zazen_shell* shell;
  struct wl_global* global;

  shell = zalloc(sizeof *shell);
  if (shell == NULL) goto out;

  global = wl_global_create(display, &z11_shell_interface, 1, shell,
                            zazen_shell_bind);
  if (global == NULL) goto out;

  return shell;

out:
  return NULL;
}
