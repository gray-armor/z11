#ifndef Z11_LIBZ11_H
#define Z11_LIBZ11_H

#include <wayland-server.h>

// compositor.h

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

// render_block.h

namespace z11 {

class RenderBlock
{
 public:
  static RenderBlock *Create(struct wl_client *client, uint32_t id, Compositor *compositor);
  ~RenderBlock();

  void Attach(struct wl_client *client, struct wl_resource *raw_buffer_resource);

  const char *sample_attr;
  struct wl_list link_;

 private:
  bool created_;
  struct wl_resource *raw_buffer_resource_;

  RenderBlock(struct wl_client *client, uint32_t id, Compositor *compositor);
};

#pragma GCC diagnostic ignored "-Wunused-parameter"
inline void RenderBlock::Attach(struct wl_client *client, struct wl_resource *raw_buffer_resource)
{
  raw_buffer_resource_ = raw_buffer_resource;
}

}  // namespace z11

#endif  // Z11_LIBZ11_H
