#ifndef Z11_COMPOSITOR_H
#define Z11_COMPOSITOR_H

#include <wayland-server.h>

namespace z11 {

class Compositor
{
 public:
  static Compositor *Create();
  ~Compositor();

  void ProcessEvents();
  struct wl_display *GetDisplay();
  struct wl_list *GetRenderBlocks();
  void InsertRenderBlock(struct wl_list *render_block);

 private:
  bool created_;
  struct wl_display *display_;
  struct wl_event_loop *loop_;
  struct wl_list render_blocks_;

  Compositor();
};

inline struct wl_display *Compositor::GetDisplay() { return display_; }

inline struct wl_list *Compositor::GetRenderBlocks() { return &this->render_blocks_; }

inline void Compositor::ProcessEvents()
{
  wl_display_flush_clients(display_);
  wl_event_loop_dispatch(loop_, -1);
}

inline void Compositor::InsertRenderBlock(struct wl_list *render_block)
{
  wl_list_insert(&render_blocks_, render_block);
}

}  // namespace z11

#endif  // Z11_COMPOSITOR_H
