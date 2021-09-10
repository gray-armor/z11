#include "virtual_object.h"

#include <wayland-server.h>

#include "callback.h"
#include "util.h"
#include "z11-server-protocol.h"

static void zazen_virtual_object_destroy(
    struct zazen_virtual_object *virtual_object);

static void zazen_virtual_object_handle_destroy(struct wl_resource *resource)
{
  struct zazen_virtual_object *virtual_object;

  virtual_object = wl_resource_get_user_data(resource);

  zazen_virtual_object_destroy(virtual_object);
}

static void zazen_virtual_object_protocol_destroy(struct wl_client *client,
                                                  struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void zazen_virtual_object_protocol_commit(struct wl_client *client,
                                                 struct wl_resource *resource)
{
  UNUSED(client);
  struct zazen_virtual_object *virtual_object;

  virtual_object = wl_resource_get_user_data(resource);

  wl_list_insert_list(&virtual_object->frame_callback_list,
                      &virtual_object->pending_frame_callback_list);
  wl_list_init(&virtual_object->pending_frame_callback_list);

  wl_signal_emit(&virtual_object->commit_signal, virtual_object);
}
static void zazen_virtual_object_protocol_frame(struct wl_client *client,
                                                struct wl_resource *resource,
                                                uint32_t callback_id)
{
  struct zazen_virtual_object *virtual_object;
  struct zazen_callback *callback;

  virtual_object = wl_resource_get_user_data(resource);

  callback = zazen_callback_create(client, callback_id);
  if (callback == NULL) {
    zazen_log("Failed to create frame callback\n");
  }

  wl_list_insert(&virtual_object->pending_frame_callback_list, &callback->link);
}

static const struct z11_virtual_object_interface
    zazen_virtual_object_interface = {
        .destroy = zazen_virtual_object_protocol_destroy,
        .commit = zazen_virtual_object_protocol_commit,
        .frame = zazen_virtual_object_protocol_frame,
};

static void zazen_virtual_object_compositor_frame_signal_handler(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  UNUSED(listener);
  struct zazen_virtual_object *virtual_object;
  struct zazen_callback *tmp;
  struct zazen_callback *frame_callback;

  virtual_object = wl_container_of(listener, virtual_object,
                                   component_frame_signal_listener);

  wl_list_for_each_safe(frame_callback, tmp,
                        &virtual_object->frame_callback_list, link)
      zazen_callback_done_with_current_time(frame_callback);

  wl_list_init(&virtual_object->frame_callback_list);
}

struct zazen_virtual_object *zazen_virtual_object_create(
    struct wl_client *client, uint32_t id, struct zazen_compositor *compositor)
{
  struct zazen_virtual_object *virtual_object;
  struct wl_resource *resource;

  virtual_object = zalloc(sizeof *virtual_object);
  if (virtual_object == NULL) {
    wl_client_post_no_memory(client);
    goto out;
  }

  resource = wl_resource_create(client, &z11_virtual_object_interface, 1, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    goto out_virtual_object;
  }

  wl_resource_set_implementation(resource, &zazen_virtual_object_interface,
                                 virtual_object,
                                 zazen_virtual_object_handle_destroy);

  virtual_object->resource = resource;

  wl_signal_init(&virtual_object->destroy_signal);
  wl_signal_init(&virtual_object->commit_signal);

  virtual_object->component_frame_signal_listener.notify =
      zazen_virtual_object_compositor_frame_signal_handler;
  wl_signal_add(&compositor->frame_signal,
                &virtual_object->component_frame_signal_listener);

  wl_list_init(&virtual_object->pending_frame_callback_list);
  wl_list_init(&virtual_object->frame_callback_list);

  return virtual_object;

out_virtual_object:
  free(virtual_object);

out:
  return NULL;
}

static void zazen_virtual_object_destroy(
    struct zazen_virtual_object *virtual_object)
{
  wl_signal_emit(&virtual_object->destroy_signal, virtual_object);
  wl_list_remove(&virtual_object->component_frame_signal_listener.link);
  free(virtual_object);
}
