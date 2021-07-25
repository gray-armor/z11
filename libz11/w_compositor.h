#ifndef Z11_W_COMPOSITOR_H
#define Z11_W_COMPOSITOR_H

#include <wayland-server.h>

namespace z11 {

class WCompositor
{
 public:
  static WCompositor *Create(struct wl_display *display);

  void Bind(struct wl_client *client, uint32_t version, uint32_t id);
  void CreateSurface(struct wl_client *client, uint32_t id);
  void CreateRegion(struct wl_client *client, uint32_t id);

 private:
  bool created_;

 private:
  WCompositor(struct wl_display *display);
};

}  // namespace z11

#endif  // Z11_W_COMPOSITOR_H
