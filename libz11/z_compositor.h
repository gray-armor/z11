#ifndef Z11_Z_COMPOSITOR_H
#define Z11_Z_COMPOSITOR_H

#include <libz11.h>
#include <wayland-server.h>

namespace z11 {

class ZCompositor
{
 public:
  static ZCompositor *Create(Compositor *compositor);

  Compositor *GetCompositor();
  void Bind(struct wl_client *client, uint32_t version, uint32_t id);
  void CreateRenderBlock(struct wl_client *client, uint32_t id);

 private:
  bool created_;
  Compositor *compositor_;

  ZCompositor(Compositor *compositor);
};

}  // namespace z11

#endif  // Z11_Z_COMPOSITOR_H
