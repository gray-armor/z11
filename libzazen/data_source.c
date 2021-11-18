#include "data_source.h"

#include <wayland-server.h>

#include "util.h"
#include "z11-server-protocol.h"

static void zazen_data_source_destroy(struct zazen_data_source* data_source);

static void zazen_data_source_handle_destroy(struct wl_resource* resource)
{
  struct zazen_data_source* data_source = wl_resource_get_user_data(resource);

  zazen_data_source_destroy(data_source);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
static void zazen_data_source_protocol_offer(struct wl_client* client,
                                             struct wl_resource* resource,
                                             const char* type)
{}

static void zazen_data_source_protocol_set_actions(struct wl_client* client,
                                                   struct wl_resource* resource,
                                                   uint32_t dnd_actions)
{}

static void zazen_data_source_protocol_destroy(struct wl_client* client,
                                               struct wl_resource* resource)
{}
#pragma GCC diagnostic pop

static const struct z11_data_source_interface zazen_data_source_interface = {
    .offer = zazen_data_source_protocol_offer,
    .set_actions = zazen_data_source_protocol_set_actions,
    .destroy = zazen_data_source_protocol_destroy,
};

struct zazen_data_source* zazen_data_source_create(struct wl_client* client,
                                                   struct wl_resource* resource,
                                                   uint32_t id)
{
  struct zazen_data_source* data_source;

  data_source = zalloc(sizeof *data_source);
  if (data_source == NULL) goto no_mem_data_source;

  data_source->resource =
      wl_resource_create(client, &z11_data_source_interface,
                         wl_resource_get_version(resource), id);
  if (data_source->resource == NULL) goto no_mem_resource;

  wl_resource_set_implementation(data_source->resource,
                                 &zazen_data_source_interface, data_source,
                                 zazen_data_source_handle_destroy);

  return data_source;

no_mem_resource:
  free(data_source);

no_mem_data_source:
  wl_client_post_no_memory(client);

  return NULL;
}

static void zazen_data_source_destroy(struct zazen_data_source* data_source)
{
  free(data_source);
}
