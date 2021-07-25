#ifndef LIBZ11_H
#define LIBZ11_H

#include <GL/glew.h>
#include <wayland-server.h>

#include "z11/list.h"

// compositor.h

namespace z11 {

class RenderBlock;

class Compositor
{
 public:
  static Compositor *Create();
  ~Compositor();

  void ProcessEvents();
  void PushRenderBlock(List<RenderBlock> *link);
  struct wl_display *display();
  List<RenderBlock> *render_block_list();

 private:
  bool created_;
  struct wl_display *display_;
  struct wl_event_loop *loop_;
  List<RenderBlock> *render_block_list_;

 private:
  Compositor();
};

inline struct wl_display *Compositor::display() { return display_; }

inline List<RenderBlock> *Compositor::render_block_list() { return this->render_block_list_; }

inline void Compositor::ProcessEvents()
{
  wl_display_flush_clients(display_);
  wl_event_loop_dispatch(loop_, 0);
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
  void Commit(struct wl_client *client);
  int32_t GetDataSize();
  void *GetData();
  GLuint vertex_array_object();

 private:
  bool created_;
  struct wl_resource *raw_buffer_resource_;
  List<RenderBlock> *link_;
  GLuint vertex_array_object_;
  GLuint vertex_buffer_;

 private:
  void Rebind();
  RenderBlock(struct wl_client *client, uint32_t id, Compositor *compositor);
};

inline GLuint RenderBlock::vertex_array_object() { return vertex_array_object_; }

}  // namespace z11

#endif  // LIBZ11_H
