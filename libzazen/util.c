#include "util.h"

#include <stdio.h>
#include <wayland-server.h>

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
