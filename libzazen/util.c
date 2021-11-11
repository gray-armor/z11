#define _GNU_SOURCE 1

#include "util.h"

#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-server.h>

int create_shared_file(off_t size, void* content)
{
  const char* name = "zazen-shared";
  int fd;
  void* data;

  fd = memfd_create(name, MFD_CLOEXEC | MFD_ALLOW_SEALING);
  if (fd < 0) return fd;
  unlink(name);

  if (ftruncate(fd, size) < 0) {
    close(fd);
    return -1;
  }

  data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED) {
    close(fd);
    return -1;
  }

  memcpy(data, content, size);

  munmap(data, size);

  return fd;
}

static void zazen_weak_pointer_handle_destroy_signal_handler(
    struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zazen_weak_ref* ref;

  ref = wl_container_of(listener, ref, destroy_listener);
  if (ref->destroy_func) ref->destroy_func(ref->data);

  wl_list_remove(&ref->destroy_listener.link);
  wl_list_init(&ref->destroy_listener.link);
  ref->data = NULL;
  ref->destroy_func = NULL;
}

void zazen_weak_ref_init(struct zazen_weak_ref* ref)
{
  wl_list_init(&ref->destroy_listener.link);
  ref->destroy_listener.notify =
      zazen_weak_pointer_handle_destroy_signal_handler;
  ref->data = NULL;
  ref->destroy_func = NULL;
}

void zazen_weak_ref_destroy(struct zazen_weak_ref* ref)
{
  wl_list_remove(&ref->destroy_listener.link);
}

void zazen_weak_ref_set_data(struct zazen_weak_ref* ref, void* data,
                             struct wl_signal* destroy_signal,
                             zazen_weak_ref_destroy_func_t on_destroy)
{
  wl_list_remove(&ref->destroy_listener.link);
  wl_signal_add(destroy_signal, &ref->destroy_listener);
  ref->data = data;
  ref->destroy_func = on_destroy;
}

void zazen_log(const char* fmt, ...)
{
  va_list argp;

  va_start(argp, fmt);
  vfprintf(stderr, fmt, argp);
  va_end(argp);
}
