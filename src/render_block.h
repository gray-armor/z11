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

  const char *sample_attr;
  struct wl_list link_;

 private:
  bool created_;

  RenderBlock(struct wl_client *client, uint32_t id, ZCompositor *z_compositor);
};

}  // namespace z11

#endif  // Z11_RENDER_BLOCK_H
