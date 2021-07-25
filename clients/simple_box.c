#define _GNU_SOURCE 1

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>
#include <wayland-client.h>

#include "z11-client-protocol.h"

struct z11_compositor *compositor = NULL;
struct wl_shm *shm = NULL;

#pragma GCC diagnostic ignored "-Wunused-parameter"
void shm_format(void *data, struct wl_shm *wl_shm, uint32_t format) {}

struct wl_shm_listener shm_listener = {
    shm_format,
};

int create_shared_fd(off_t size)
{
  char name[1024] = "";

  int fd = memfd_create("z11-simple-box", MFD_CLOEXEC | MFD_ALLOW_SEALING);
  if (fd < 0) {
    fprintf(stderr, "File cannot open: %s\n", name);
    exit(1);
  } else {
    unlink(name);
  }

  if (ftruncate(fd, size) < 0) {
    fprintf(stderr, "ftruncate failed: fd=%i, size=%li\n", fd, size);
    close(fd);
    exit(1);
  }

  return fd;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
void global_registry_handler(void *data, struct wl_registry *registry, uint32_t id, const char *interface,
                             uint32_t version)
{
  if (strcmp(interface, "z11_compositor") == 0) {
    compositor = wl_registry_bind(registry, id, &z11_compositor_interface, version);
  } else if (strcmp(interface, "wl_shm") == 0) {
    shm = wl_registry_bind(registry, id, &wl_shm_interface, 1);
    wl_shm_add_listener(shm, &shm_listener, NULL);
  }
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
void global_registry_remover(void *data, struct wl_registry *registry, uint32_t id) {}

const struct wl_registry_listener registry_listener = {
    global_registry_handler,
    global_registry_remover,
};

typedef struct {
  float x, y, z;
} Point;

typedef struct {
  Point start, end;
} Line;

static void paint(Line *out, float x, float y, float z, float theta)
{
  double root2x5 = sqrt(2) * 5;
  double root2x5_cos = root2x5 * cos(theta + M_PI / 4);
  double root2x5_sin = root2x5 * sin(theta + M_PI / 4);

  Point A = {x - root2x5_cos, y - 5, z - root2x5_sin};
  Point B = {x + root2x5_sin, y - 5, z - root2x5_cos};
  Point C = {x + root2x5_sin, y + 5, z - root2x5_cos};
  Point D = {x - root2x5_cos, y + 5, z - root2x5_sin};
  Point E = {x - root2x5_sin, y - 5, z + root2x5_cos};
  Point F = {x + root2x5_cos, y - 5, z + root2x5_sin};
  Point G = {x + root2x5_cos, y + 5, z + root2x5_sin};
  Point H = {x - root2x5_sin, y + 5, z + root2x5_cos};

  out->start = A;
  out->end = B;
  out++;
  out->start = B;
  out->end = C;
  out++;
  out->start = C;
  out->end = D;
  out++;
  out->start = D;
  out->end = A;
  out++;

  out->start = E;
  out->end = F;
  out++;
  out->start = F;
  out->end = G;
  out++;
  out->start = G;
  out->end = H;
  out++;
  out->start = H;
  out->end = E;
  out++;

  out->start = A;
  out->end = E;
  out++;
  out->start = B;
  out->end = F;
  out++;
  out->start = C;
  out->end = G;
  out++;
  out->start = D;
  out->end = H;
}

int main(int argc, char const *argv[])
{
  int x = 0;
  int y = 0;
  int z = 20;
  double del_theta = 0;
  double theta = 0;
  if (argc > 1) x = atoi(argv[1]);
  if (argc > 2) y = atoi(argv[2]);
  if (argc > 3) z = atoi(argv[3]);
  if (argc > 4) del_theta = atof(argv[4]) / 100;

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

  assert(compositor && shm);

  int size = sizeof(Line) * 12;
  int fd = create_shared_fd(size);
  Line *shm_data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (shm_data == MAP_FAILED) {
#pragma GCC diagnostic ignored "-Wformat"
    fprintf(stderr, "mmap failed: %m\n");
    close(fd);
    exit(1);
  }
  paint(shm_data, x, y, z, theta);

  struct wl_shm_pool *pool = wl_shm_create_pool(shm, fd, size);
  struct wl_raw_buffer *buffer = wl_shm_pool_create_raw_buffer(pool, 0, size);
  wl_shm_pool_destroy(pool);

  struct z11_render_block *render_block = z11_compositor_create_render_block(compositor);
  z11_render_block_attach(render_block, buffer);
  z11_render_block_commit(render_block);

  struct timeval base, now;
  gettimeofday(&base, NULL);

  int ret;
  while (wl_display_dispatch_pending(display) != -1) {
    ret = wl_display_flush(display);
    if (ret == -1) break;

    gettimeofday(&now, NULL);
    if ((now.tv_sec - base.tv_sec) * 1000000 + now.tv_usec - base.tv_usec > 16666) {  // 60 hz
      theta += del_theta;
      if (theta >= 2 * M_PI || theta <= -2 * M_PI) theta = 0;
      base = now;
      paint(shm_data, x, y, z, theta);
      z11_render_block_commit(render_block);
    }
  }

  return 0;
}
