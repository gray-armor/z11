#ifndef LIBZAZEN_TYPED_H
#define LIBZAZEN_TYPED_H

#include <wayland-server.h>

typedef union {
  wl_fixed_t fixed;
  float flt;
} fixed_float;

#endif  //  LIBZAZEN_TYPED_H
