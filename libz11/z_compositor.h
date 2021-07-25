#ifndef LIBZ11_Z_COMPOSITOR_H
#define LIBZ11_Z_COMPOSITOR_H

#include <libz11.h>
#include <wayland-server.h>

namespace z11 {

class ZCompositor
{
 public:
  static ZCompositor *Create(Compositor *compositor);

  void Bind(struct wl_client *client, uint32_t version, uint32_t id);
  void CreateRenderBlock(struct wl_client *client, uint32_t id);
  Compositor *compositor();

 private:
  bool created_;
  Compositor *compositor_;

 private:
  ZCompositor(Compositor *compositor);
};

inline Compositor *ZCompositor::compositor() { return compositor_; }

}  // namespace z11

#endif  // LIBZ11_Z_COMPOSITOR_H
