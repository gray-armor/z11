#ifndef Z11_Z_SERVER_H
#define Z11_Z_SERVER_H

#include <libzazen.h>
#include <wayland-server.h>

class ZServer
{
 public:
  bool Init();
  void Poll();

  class RenderElementIterator
  {
   public:
    RenderElementIterator(struct wl_list *list);
    struct zazen_render_element *Next();
    void Rewind();

   private:
    struct wl_list *list_;
    struct wl_list *pos_;
  };

  RenderElementIterator *NewRenderElementIterator();
  void DeleteRenderElementIterator(RenderElementIterator *render_element_iterator);

 private:
  struct zazen_compositor *compositor_;
  struct wl_display *display_;
  struct wl_event_loop *loop_;
};

#endif  // Z11_Z_SERVER_H
