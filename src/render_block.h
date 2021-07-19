#ifndef Z11_Z_RENDER_BLOCK_H
#define Z11_Z_RENDER_BLOCK_H

#include <wayland-server-core.h>

#include "z_compositor.h"

namespace z11 {

class RenderBlock
{
 public:
  static RenderBlock *Create(struct wl_client *client, uint32_t id, ZCompositor *z_compositor);
  ~RenderBlock();

  void Attach(struct wl_client *client, struct wl_resource *raw_buffer_resource);

  const char *sample_attr;
  struct wl_list link_;

 private:
  bool created_;
  struct wl_resource *raw_buffer_resource_;

  RenderBlock(struct wl_client *client, uint32_t id, ZCompositor *z_compositor);
};

#pragma GCC diagnostic ignored "-Wunused-parameter"
inline void RenderBlock::Attach(struct wl_client *client, struct wl_resource *raw_buffer_resource)
{
  raw_buffer_resource_ = raw_buffer_resource;
}

}  // namespace z11

#endif  // Z11_RENDER_BLOCK_H
