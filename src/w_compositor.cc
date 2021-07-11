#include "w_compositor.h"

#include <wayland-server.h>

#include "wayland-server-core.hh"
#include "wayland-server-protocol.hh"

namespace z11 {

WCompositor *WCompositor::Create(struct wl_display *display)
{
  WCompositor *w_compositor = new WCompositor(display);
  if (w_compositor->created_) return w_compositor;
  delete w_compositor;
  return nullptr;
}

WCompositor::WCompositor(struct wl_display *display) : created_(false)
{
  struct wl_global *global;
  global = wl_cpp::global_create(display, &wl_compositor_interface, 4, this);
  if (global == nullptr) return;

  created_ = true;
  return;
}

void WCompositor::Bind(struct wl_client *client, uint32_t version, uint32_t id)
{
  struct wl_resource *resource;
  resource = wl_resource_create(client, &wl_compositor_interface, version, id);
  if (resource == nullptr) goto no_mem_resource;

  wl_cpp::compositor_resource_bind(resource, this);

  return;

no_mem_resource:
  wl_client_post_no_memory(client);
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
void WCompositor::CreateSurface(struct wl_client *client, uint32_t id) {}
#pragma GCC diagnostic ignored "-Wunused-parameter"
void WCompositor::CreateRegion(struct wl_client *client, uint32_t id) {}

}  // namespace z11
