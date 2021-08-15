#ifndef LIBZAZEN_COMPOSITOR_H
#define LIBZAZEN_COMPOSITOR_H

#include <wayland-server.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
struct zazen_compositor {};
#pragma GCC diagnostic pop

struct zazen_compositor* zazen_compositor_create(struct wl_display* display);

#endif  //  LIBZAZEN_COMPOSITOR_H
