#ifndef Z11_CLIENT_HELPER_H
#define Z11_CLIENT_HELPER_H

#include <stdlib.h>
#include <wayland-client.h>

#include "z11-opengl-client-protocol.h"

typedef struct {
  float x, y, z;
} Point;

typedef struct {
  Point p1, p2, p3;
} Face;

typedef struct {
  float u, v;
} UVCoord;

typedef struct {
  Point s, e;
} Line;

typedef struct {
  uint8_t a, r, g, b;
} ColorARGB;

typedef struct {
  uint8_t b, g, r, a;
} ColorBGRA;

typedef struct {
  uint8_t r, g, b, a;
} ColorRGBA;

typedef struct {
  uint8_t r, g, b;
} ColorRGB;

struct z11_global {
  struct z11_compositor *compositor;
  struct wl_shm *shm;
  struct z11_opengl *gl;
  struct z11_opengl_render_component_manager *render_component_manager;
  struct wl_display *display;
};

struct z11_global *z_helper_global();

int create_shared_fd(off_t size);

void print_fps(int interval_sec);

unsigned char *z_helper_png(const char *filename, __uint32_t *width,
                            __uint32_t *height, __uint32_t *ch);

Face *z_helper_stl(const char *filename, int *face_count);

#endif  // Z11_CLIENT_HELPER_H
