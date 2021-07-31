#ifndef Z11_Z_SERVER_H
#define Z11_Z_SERVER_H

#include <libz11.h>
#include <wayland-server.h>

class ZServer
{
 public:
  bool Init();
  void Poll();

  class RenderBlockIterator
  {
   public:
    RenderBlockIterator(struct wl_list *list);
    struct z_render_block *Next();
    void Rewind();

   private:
    struct wl_list *list_;
    struct wl_list *pos_;
  };

  RenderBlockIterator *NewRenderBlockIterator();
  void DeleteRenderBlockIterator(RenderBlockIterator *render_block_iterator);

 private:
  struct z_compositor *compositor_;
  struct wl_display *display_;
  struct wl_event_loop *loop_;
};

#endif  // Z11_Z_SERVER_H
