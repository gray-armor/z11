#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client.h>

#include "z11-client-protocol.h"

struct z11_compositor *compositor = NULL;

#pragma GCC diagnostic ignored "-Wunused-parameter"
void global_registry_handler(void *data, struct wl_registry *registry, uint32_t id, const char *interface,
                             uint32_t version)
{
  if (strcmp(interface, "z11_compositor") == 0) {
    compositor = wl_registry_bind(registry, id, &z11_compositor_interface, version);
    z11_compositor_create_render_block(compositor);
  }
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
void global_registry_remover(void *data, struct wl_registry *registry, uint32_t id) {}

const struct wl_registry_listener registry_listener = {
    global_registry_handler,
    global_registry_remover,
};

int main()
{
  struct wl_display *display = wl_display_connect(NULL);
  if (display == NULL) {
    fprintf(stderr, "Can't connect to display\n");
    exit(1);
  }
  fprintf(stdout, "connected to display\n");

  struct wl_registry *registry = wl_display_get_registry(display);
  wl_registry_add_listener(registry, &registry_listener, NULL);

  wl_display_dispatch(display);
  wl_display_roundtrip(display);

  while (wl_display_dispatch(display) != -1) {
    ;
  }

  return 0;
}
