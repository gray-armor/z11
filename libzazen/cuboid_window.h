#ifndef LIBZAZEN_CUBOID_WINDOW
#define LIBZAZEN_CUBOID_WINDOW

#include <wayland-server.h>

struct zazen_cuboid_window {
  uint32_t width;
  uint32_t height;
  uint32_t depth;
};

struct zazen_cuboid_window* zazen_cuboid_window_create(struct wl_client* client,
                                                       uint32_t id);

#endif  //  LIBZAZEN_CUBOID_WINDOW
