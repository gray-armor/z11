#include "opengl_render_component_manager.h"

#include <libzazen.h>
#include <wayland-server.h>

#include "opengl_render_component.h"
#include "util.h"
#include "z11-opengl-server-protocol.h"

static void
zazen_opengl_render_component_manager_protocol_create_render_component(
    struct wl_client* client, struct wl_resource* resource, uint32_t id,
    struct wl_resource* virtual_object_resource)
{
  struct zazen_opengl_render_component* render_component;
  struct zazen_opengl_render_component_manager* render_component_manager;
  struct zazen_virtual_object* virtual_object;

  render_component_manager = wl_resource_get_user_data(resource);

  virtual_object = wl_resource_get_user_data(virtual_object_resource);

  render_component = zazen_opengl_render_component_create(
      client, id, render_component_manager, virtual_object);
  if (render_component == NULL) {
    zazen_log("Failed to create a render component\n");
  }
}

static const struct z11_opengl_render_component_manager_interface
    zazen_opengl_render_component_manager_interface = {
        .create_opengl_render_component =
            zazen_opengl_render_component_manager_protocol_create_render_component,
};

static void zazen_opengl_render_component_manager_bind(struct wl_client* client,
                                                       void* data,
                                                       uint32_t version,
                                                       uint32_t id)
{
  struct zazen_opengl_render_component_manager* render_component_manager = data;
  struct wl_resource* resource;

  resource = wl_resource_create(
      client, &z11_opengl_render_component_manager_interface, version, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    return;
  }

  wl_resource_set_implementation(
      resource, &zazen_opengl_render_component_manager_interface,
      render_component_manager, NULL);
}

struct wl_list*
zazen_opengl_render_component_manager_get_render_component_back_state_list(
    struct zazen_opengl_render_component_manager* manager)
{
  return &manager->render_component_back_state_list;
}

struct zazen_opengl_render_component_manager*
zazen_opengl_render_component_manager_create(struct wl_display* display)
{
  struct zazen_opengl_render_component_manager* render_component_manager;

  render_component_manager = zalloc(sizeof *render_component_manager);
  if (render_component_manager == NULL) return NULL;

  if (wl_global_create(display, &z11_opengl_render_component_manager_interface,
                       1, render_component_manager,
                       zazen_opengl_render_component_manager_bind) == NULL) {
    free(render_component_manager);
    return NULL;
  }

  wl_list_init(&render_component_manager->render_component_back_state_list);

  return render_component_manager;
}
