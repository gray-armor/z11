#include "callback.h"

#include <sys/time.h>
#include <wayland-server.h>

#include "util.h"

static void zazen_callback_destroy(struct zazen_callback *callback);

static void zazen_callback_handle_destroy(struct wl_resource *resource)
{
  struct zazen_callback *callback;

  callback = wl_resource_get_user_data(resource);

  zazen_callback_destroy(callback);
}

void zazen_callback_done_with_current_time(struct zazen_callback *callback)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);

  uint32_t current_time_in_millis = tv.tv_sec * 1000 + tv.tv_usec / 1000;

  wl_callback_send_done(callback->resource, current_time_in_millis);
  wl_resource_destroy(callback->resource);
}

struct zazen_callback *zazen_callback_create(struct wl_client *client, uint32_t id)
{
  struct zazen_callback *callback;
  struct wl_resource *resource;

  callback = zalloc(sizeof *callback);
  if (callback == NULL) {
    wl_client_post_no_memory(client);
    goto out;
  }

  resource = wl_resource_create(client, &wl_callback_interface, 1, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    goto out_callback;
  }

  callback->resource = resource;
  wl_list_init(&callback->link);

  wl_resource_set_implementation(resource, NULL, callback, zazen_callback_handle_destroy);

  return callback;

out_callback:
  free(callback);

out:
  return NULL;
}

static void zazen_callback_destroy(struct zazen_callback *callback)
{
  wl_list_remove(&callback->link);
  free(callback);
}
