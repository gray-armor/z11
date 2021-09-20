#ifndef Z11_CLIENT_Z_WINDOW_H
#define Z11_CLIENT_Z_WINDOW_H

#include <wayland-client.h>
#include <z11-client-protocol.h>
#include <z11-opengl-client-protocol.h>

class ZWindow
{
 public:
  bool Connect(const char *socket);
  void GlobalRegistryHandler(struct wl_registry *registry, uint32_t id,
                             const char *interface, uint32_t version);
  void GlobalRegistryRemover(struct wl_registry *registry, uint32_t id);
  void ShmFormat(struct wl_shm *wl_shm, uint32_t format);
  int CreateSharedFD(off_t size);
  inline struct wl_display *display();
  inline struct wl_registry *registry();
  inline struct z11_compositor *compositor();
  inline struct wl_shm *shm();
  inline struct z11_shell *shell();
  inline struct z11_opengl *gl();
  inline struct z11_opengl_render_component_manager *render_component_manager();

 private:
  struct wl_display *display_;
  struct wl_registry *registry_;
  struct z11_compositor *compositor_;
  struct wl_shm *shm_;
  struct z11_shell *shell_;
  struct z11_opengl *gl_;
  struct z11_seat *seat_;
  struct z11_opengl_render_component_manager *render_component_manager_;
};

inline struct wl_display *ZWindow::display() { return display_; }
inline struct wl_registry *ZWindow::registry() { return registry_; }
inline struct z11_compositor *ZWindow::compositor() { return compositor_; }
inline struct wl_shm *ZWindow::shm() { return shm_; }
inline struct z11_shell *ZWindow::shell() { return shell_; }
inline struct z11_opengl *ZWindow::gl() { return gl_; }
inline struct z11_opengl_render_component_manager *
ZWindow::render_component_manager()
{
  return render_component_manager_;
}
#endif  //  Z11_CLIENT_Z_WINDOW_H
