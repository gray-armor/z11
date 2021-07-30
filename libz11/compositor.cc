#include <libz11.h>

#include "z_compositor.h"

namespace z11 {

Compositor* Compositor::Create()
{
  Compositor* compositor = new Compositor();
  if (compositor->created_) return compositor;
  delete compositor;
  return nullptr;
}

Compositor::Compositor() : created_(false), display_(nullptr), loop_(nullptr), render_block_list_(nullptr)
{
  const char* socket;
  display_ = wl_display_create();
  loop_ = wl_display_get_event_loop(display_);
  render_block_list_ = new List<RenderBlock>();

  ZCompositor* z_compositor = ZCompositor::Create(this);
  if (z_compositor == nullptr) return;

  wl_display_init_shm(this->display_);

  socket = wl_display_add_socket_auto(display_);
  if (socket == nullptr) return;

  created_ = true;
  return;
}

Compositor::~Compositor() { wl_display_destroy(display_); }

void Compositor::PushRenderBlock(List<RenderBlock>* link) { render_block_list_->Insert(link); }

}  // namespace z11
