#include "z_server.h"

#include <libz11.h>
#include <wayland-server.h>

bool ZServer::Init()
{
  const char* socket;
  struct z_gl* gl;

  display_ = wl_display_create();
  loop_ = wl_display_get_event_loop(display_);

  compositor_ = z_compositor_create(display_);
  if (compositor_ == NULL) return false;

  gl = z_gl_create(display_);
  if (gl == NULL) return false;

  wl_display_init_shm(display_);

  socket = wl_display_add_socket_auto(display_);
  if (socket == NULL) return false;

  return true;
}

void ZServer::Poll()
{
  wl_display_flush_clients(display_);
  wl_event_loop_dispatch(loop_, 0);
}

ZServer::RenderBlockIterator* ZServer::NewRenderBlockIterator()
{
  return new RenderBlockIterator(z_compositor_get_render_block_list(compositor_));
}

void ZServer::DeleteRenderBlockIterator(RenderBlockIterator* render_block_iterator)
{
  delete render_block_iterator;
}

ZServer::RenderBlockIterator::RenderBlockIterator(struct wl_list* list) : list_(list), pos_(list) {}

struct z_render_block* ZServer::RenderBlockIterator::Next()
{
  if (pos_->next == list_) return nullptr;
  pos_ = pos_->next;
  return z_render_block_from_link(pos_);
}

void ZServer::RenderBlockIterator::Rewind() { pos_ = list_; }
