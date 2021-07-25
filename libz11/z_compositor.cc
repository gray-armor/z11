#include "z_compositor.h"

#include <libz11.h>

#include "wayland-server-core.hh"
#include "z11-server-protocol.h"
#include "z11-server-protocol.hh"

namespace z11 {
ZCompositor* ZCompositor::Create(Compositor* compositor)
{
  ZCompositor* z_compositor = new ZCompositor(compositor);
  if (z_compositor->created_) return z_compositor;
  delete z_compositor;
  return nullptr;
}

ZCompositor::ZCompositor(Compositor* compositor) : created_(false), compositor_(compositor)
{
  struct wl_global* global;
  global = wl_cpp::global_create(compositor->display(), &z11_compositor_interface, 1, this);
  if (global == nullptr) return;

  created_ = true;
  return;
}

void ZCompositor::Bind(struct wl_client* client, uint32_t version, uint32_t id)
{
  struct wl_resource* resource;
  resource = wl_resource_create(client, &z11_compositor_interface, version, id);
  if (resource == nullptr) goto no_mem_resource;

  z_cpp::z_compositor_resource_bind_no_destroy(resource, this);

  return;

no_mem_resource:
  wl_client_post_no_memory(client);
}

void ZCompositor::CreateRenderBlock(struct wl_client* client, uint32_t id)
{
  RenderBlock::Create(client, id, this->compositor());
}

}  // namespace z11
