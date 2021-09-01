#ifndef Z11_Z_SERVER_H
#define Z11_Z_SERVER_H

#include <libzazen.h>
#include <wayland-server.h>

class ZServer
{
 public:
  bool Init();
  bool InitInput();
  void Poll();
  void Frame();
  void Shutdown();

  class RenderStateIterator
  {
   public:
    RenderStateIterator(struct wl_list *render_component_back_state_list);
    struct zazen_opengl_render_component_back_state *Next();
    void Rewind();

   private:
    struct wl_list *list_;
    struct wl_list *pos_;
  };

  RenderStateIterator *NewRenderStateIterator();
  void DeleteRenderStateIterator(RenderStateIterator *render_component_iterator);

 private:
  struct zazen_compositor *compositor_;
  struct zazen_opengl_render_component_manager *render_component_manager_;
  struct wl_display *display_;
  struct wl_event_loop *loop_;
};

#endif  // Z11_Z_SERVER_H
