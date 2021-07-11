#include "render_block.h"

#include "z11-server-protocol.h"
#include "z11-server-protocol.hh"

namespace z11 {

RenderBlock* RenderBlock::Create(struct wl_client* client, uint32_t id, ZCompositor* z_compositor)
{
  RenderBlock* render_block = new RenderBlock(client, id, z_compositor);
  if (render_block->created_) return render_block;
  delete render_block;
  return nullptr;
}

RenderBlock::RenderBlock(struct wl_client* client, uint32_t id, ZCompositor* z_compositor)
    : sample_attr("Hello from Render Block"), created_(false)
{
  struct wl_resource* resource;

  z_compositor->GetCompositor()->InsertRenderBlock(&this->link_);

  resource = wl_resource_create(client, &z11_render_block_interface, 1, id);
  if (resource == nullptr) goto no_mem_resource;

  z_cpp::z_render_block_resource_bind(resource, this);

  created_ = true;
  return;

no_mem_resource:
  wl_client_post_no_memory(client);
  return;
}

RenderBlock::~RenderBlock() { wl_list_remove(&this->link_); }

}  // namespace z11
