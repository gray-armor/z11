#define _GNU_SOURCE 1

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
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

  assert(compositor && shm);

  int size = 1024;
  int fd = create_shared_fd(size);
  void *shm_data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (shm_data == MAP_FAILED) {
#pragma GCC diagnostic ignored "-Wformat"
    fprintf(stderr, "mmap failed: %m\n");
    close(fd);
    exit(1);
  }

  struct wl_shm_pool *pool = wl_shm_create_pool(shm, fd, size);
  struct wl_raw_buffer *buffer = wl_shm_pool_create_raw_buffer(pool, 0, size);
  wl_shm_pool_destroy(pool);

  struct z11_render_block *render_block = z11_compositor_create_render_block(compositor);
  z11_render_block_attach(render_block, buffer);

  // TODO: fill the buffer and commit

  while (wl_display_dispatch(display) != -1) {
    ;
  }

  return 0;
}
