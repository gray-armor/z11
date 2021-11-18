#ifndef Z11_CLIENT_Z_WINDOW_H
#define Z11_CLIENT_Z_WINDOW_H

#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>
#include <z11-client-protocol.h>
#include <z11-opengl-client-protocol.h>

typedef union {
  wl_fixed_t fixed;
  float flt;
} fixed_float;

template <class T>
class ZWindow
{
 public:
  ZWindow(T *delegate);
  bool Connect(const char *socket);
  void GlobalRegistryHandler(struct wl_registry *registry, uint32_t id,
                             const char *interface, uint32_t version);
  void GlobalRegistryRemover(struct wl_registry *registry, uint32_t id);
  void SeatCapability(struct z11_seat *seat, uint32_t capabilities);
  void HandleRayEnter(struct z11_ray *ray, uint32_t serial,
                      struct z11_cuboid_window *cuboid_window,
                      float ray_origin_x, float ray_origin_y,
                      float ray_origin_z, float ray_direction_x,
                      float ray_direction_y, float ray_direction_z);
  void HandleRayMotion(struct z11_ray *ray, uint32_t time, float ray_origin_x,
                       float ray_origin_y, float ray_origin_z,
                       float ray_direction_x, float ray_direction_y,
                       float ray_direction_z);
  void HandleRayLeave(struct z11_ray *ray, uint32_t serial,
                      struct z11_cuboid_window *cuboid_window);
  void HandleRayButton(struct z11_ray *ray, uint32_t serial, uint32_t time,
                       uint32_t button, uint32_t state);
  void HandleKeyboardKeymap(struct z11_keyboard *keyboard, uint32_t format,
                            int fd, uint32_t size);
  void HandleKeyboardEnter(struct z11_keyboard *keyboard, uint32_t serial,
                           struct z11_cuboid_window *cuboid_window,
                           struct wl_array *keys);
  void HandleKeyboardLeave(struct z11_keyboard *keyboard, uint32_t serial,
                           struct z11_cuboid_window *cuboid_window);
  void HandleKeyboardKey(struct z11_keyboard *keyboard, uint32_t serial,
                         uint32_t time, uint32_t key, uint32_t state);
  void HandleKeyboardModifiers(struct z11_keyboard *keyboard, uint32_t serial,
                               uint32_t mods_depressed, uint32_t mods_latched,
                               uint32_t mods_locked, uint32_t group);
  void ShmFormat(struct wl_shm *wl_shm, uint32_t format);
  int CreateSharedFD(off_t size);
  inline struct wl_display *display();
  inline struct wl_registry *registry();
  inline struct z11_compositor *compositor();
  inline struct wl_shm *shm();
  inline struct z11_shell *shell();
  inline struct z11_opengl *gl();
  inline struct z11_seat *seat();
  inline struct z11_opengl_render_component_manager *render_component_manager();
  inline struct z11_data_device_manager *data_device_manager();

 private:
  T *delegate_;
  struct wl_display *display_;
  struct wl_registry *registry_;
  struct z11_compositor *compositor_;
  struct wl_shm *shm_;
  struct z11_shell *shell_;
  struct z11_opengl *gl_;
  struct z11_seat *seat_;
  struct z11_opengl_render_component_manager *render_component_manager_;
  struct z11_data_device_manager *data_device_manager_;
};

template <class T>
static void global_registry_handler(void *data, struct wl_registry *registry,
                                    uint32_t id, const char *interface,
                                    uint32_t version)
{
  ZWindow<T> *zwindow = (ZWindow<T> *)data;
  zwindow->GlobalRegistryHandler(registry, id, interface, version);
}

template <class T>
static void global_registry_remover(void *data, struct wl_registry *registry,
                                    uint32_t id)
{
  ZWindow<T> *zwindow = (ZWindow<T> *)data;
  zwindow->GlobalRegistryRemover(registry, id);
}

template <class T>
static const struct wl_registry_listener registry_listener = {
    global_registry_handler<T>,
    global_registry_remover<T>,
};

template <class T>
static void seat_capability(void *data, struct z11_seat *seat,
                            uint32_t capabilities)
{
  ZWindow<T> *zwindow = (ZWindow<T> *)data;
  zwindow->SeatCapability(seat, capabilities);
}

template <class T>
static const struct z11_seat_listener seat_listener = {
    seat_capability<T>,
};

template <class T>
static void shm_format(void *data, struct wl_shm *wl_shm, uint32_t format)
{
  ZWindow<T> *zwindow = (ZWindow<T> *)data;
  zwindow->ShmFormat(wl_shm, format);
}

template <class T>
static const struct wl_shm_listener shm_listener = {
    shm_format<T>,
};

template <class T>
static void handle_ray_enter(void *data, struct z11_ray *ray, uint32_t serial,
                             struct z11_cuboid_window *cuboid_window,
                             wl_fixed_t ray_origin_x, wl_fixed_t ray_origin_y,
                             wl_fixed_t ray_origin_z,
                             wl_fixed_t ray_direction_x,
                             wl_fixed_t ray_direction_y,
                             wl_fixed_t ray_direction_z)
{
  ZWindow<T> *zwindow = (ZWindow<T> *)data;
  fixed_float origin_x, origin_y, origin_z, direction_x, direction_y,
      direction_z;

  origin_x.fixed = ray_origin_x;
  origin_y.fixed = ray_origin_y;
  origin_z.fixed = ray_origin_z;
  direction_x.fixed = ray_direction_x;
  direction_y.fixed = ray_direction_y;
  direction_z.fixed = ray_direction_z;

  zwindow->HandleRayEnter(ray, serial, cuboid_window, origin_x.flt,
                          origin_y.flt, origin_z.flt, direction_x.flt,
                          direction_y.flt, direction_z.flt);
}

template <class T>
static void handle_ray_motion(void *data, struct z11_ray *ray, uint32_t time,
                              wl_fixed_t ray_origin_x, wl_fixed_t ray_origin_y,
                              wl_fixed_t ray_origin_z,
                              wl_fixed_t ray_direction_x,
                              wl_fixed_t ray_direction_y,
                              wl_fixed_t ray_direction_z)
{
  ZWindow<T> *zwindow = (ZWindow<T> *)data;
  fixed_float origin_x, origin_y, origin_z, direction_x, direction_y,
      direction_z;

  origin_x.fixed = ray_origin_x;
  origin_y.fixed = ray_origin_y;
  origin_z.fixed = ray_origin_z;
  direction_x.fixed = ray_direction_x;
  direction_y.fixed = ray_direction_y;
  direction_z.fixed = ray_direction_z;

  zwindow->HandleRayMotion(ray, time, origin_x.flt, origin_y.flt, origin_z.flt,
                           direction_x.flt, direction_y.flt, direction_z.flt);
}

template <class T>
static void handle_ray_leave(void *data, struct z11_ray *ray, uint32_t serial,
                             struct z11_cuboid_window *cuboid_window)
{
  ZWindow<T> *zwindow = (ZWindow<T> *)data;
  zwindow->HandleRayLeave(ray, serial, cuboid_window);
}

template <class T>
static void handle_ray_button(void *data, struct z11_ray *ray, uint32_t serial,
                              uint32_t time, uint32_t button, uint32_t state)
{
  ZWindow<T> *zwindow = (ZWindow<T> *)data;
  zwindow->HandleRayButton(ray, serial, time, button, state);
}

template <class T>
static const struct z11_ray_listener ray_listener = {
    handle_ray_enter<T>,
    handle_ray_leave<T>,
    handle_ray_motion<T>,
    handle_ray_button<T>,
};

template <class T>
static void handle_keyboard_keymap(void *data, struct z11_keyboard *keyboard,
                                   uint32_t format, int fd, uint32_t size)
{
  ZWindow<T> *zwindow = (ZWindow<T> *)data;
  zwindow->HandleKeyboardKeymap(keyboard, format, fd, size);
}

template <class T>
static void handle_keyboard_enter(void *data, struct z11_keyboard *keyboard,
                                  uint32_t serial,
                                  struct z11_cuboid_window *cuboid_window,
                                  struct wl_array *keys)
{
  ZWindow<T> *zwindow = (ZWindow<T> *)data;
  zwindow->HandleKeyboardEnter(keyboard, serial, cuboid_window, keys);
}

template <class T>
static void handle_keyboard_leave(void *data, struct z11_keyboard *keyboard,
                                  uint32_t serial,
                                  struct z11_cuboid_window *cuboid_window)
{
  ZWindow<T> *zwindow = (ZWindow<T> *)data;
  zwindow->HandleKeyboardLeave(keyboard, serial, cuboid_window);
}

template <class T>
static void handle_keyboard_key(void *data, struct z11_keyboard *keyboard,
                                uint32_t serial, uint32_t time, uint32_t key,
                                uint32_t state)
{
  ZWindow<T> *zwindow = (ZWindow<T> *)data;
  zwindow->HandleKeyboardKey(keyboard, serial, time, key, state);
}

template <class T>
static void handle_keyboard_modifiers(void *data, struct z11_keyboard *keyboard,
                                      uint32_t serial, uint32_t mods_depressed,
                                      uint32_t mods_latached,
                                      uint32_t mods_locked, uint32_t group)
{
  ZWindow<T> *zwindow = (ZWindow<T> *)data;
  zwindow->HandleKeyboardModifiers(keyboard, serial, mods_depressed,
                                   mods_latached, mods_locked, group);
}

template <class T>
static const struct z11_keyboard_listener keyboard_listener = {
    handle_keyboard_keymap<T>,    handle_keyboard_enter<T>,
    handle_keyboard_leave<T>,     handle_keyboard_key<T>,
    handle_keyboard_modifiers<T>,
};

template <class T>
ZWindow<T>::ZWindow(T *delegate)
{
  delegate_ = delegate;
}

template <class T>
bool ZWindow<T>::Connect(const char *socket)
{
  display_ = wl_display_connect(socket);
  if (display_ == nullptr) goto out;

  registry_ = wl_display_get_registry(display_);
  if (registry_ == nullptr) goto out_display;

  wl_registry_add_listener(registry_, &registry_listener<T>, this);

  wl_display_dispatch(display_);
  wl_display_roundtrip(display_);

  if (compositor_ == NULL || gl_ == NULL || shm_ == NULL ||
      render_component_manager_ == NULL || shell_ == NULL || seat_ == NULL)
    goto out_registry;

  return true;

out_registry:
  wl_registry_destroy(registry_);

out_display:
  wl_display_disconnect(display_);

out:
  return false;
}

template <class T>
void ZWindow<T>::GlobalRegistryHandler(struct wl_registry *registry,
                                       uint32_t id, const char *interface,
                                       uint32_t version)
{
  if (strcmp(interface, "z11_compositor") == 0) {
    compositor_ = (z11_compositor *)wl_registry_bind(
        registry, id, &z11_compositor_interface, version);
  } else if (strcmp(interface, "wl_shm") == 0) {
    shm_ = (wl_shm *)wl_registry_bind(registry, id, &wl_shm_interface, version);
    wl_shm_add_listener(shm_, &shm_listener<T>, this);
  } else if (strcmp(interface, "z11_opengl") == 0) {
    gl_ = (z11_opengl *)wl_registry_bind(registry, id, &z11_opengl_interface,
                                         version);
  } else if (strcmp(interface, "z11_opengl_render_component_manager") == 0) {
    render_component_manager_ =
        (z11_opengl_render_component_manager *)wl_registry_bind(
            registry, id, &z11_opengl_render_component_manager_interface,
            version);
  } else if (strcmp(interface, "z11_seat") == 0) {
    seat_ = (z11_seat *)wl_registry_bind(registry, id, &z11_seat_interface,
                                         version);
    z11_seat_add_listener(seat_, &seat_listener<T>, this);
  } else if (strcmp(interface, "z11_shell") == 0) {
    shell_ = (z11_shell *)wl_registry_bind(registry, id, &z11_shell_interface,
                                           version);
  } else if (strcmp(interface, "z11_data_device_manager") == 0) {
    data_device_manager_ = (z11_data_device_manager *)wl_registry_bind(
        registry, id, &z11_data_device_manager_interface, version);
  }
}

template <class T>
void ZWindow<T>::GlobalRegistryRemover(struct wl_registry *registry,
                                       uint32_t id)
{
  (void)registry;
  (void)id;
}

template <class T>
void ZWindow<T>::SeatCapability(struct z11_seat *seat, uint32_t capabilities)
{
  struct z11_ray *ray;
  struct z11_keyboard *keyboard;

  if (capabilities & Z11_SEAT_CAPABILITY_RAY) {
    ray = z11_seat_get_ray(seat);
    z11_ray_add_listener(ray, &ray_listener<T>, this);
  }
  if (capabilities & Z11_SEAT_CAPABILITY_KEYBOARD) {
    keyboard = z11_seat_get_keyboard(seat);
    z11_keyboard_add_listener(keyboard, &keyboard_listener<T>, this);
  }
}

template <class T>
void ZWindow<T>::HandleRayEnter(struct z11_ray *ray, uint32_t serial,
                                struct z11_cuboid_window *cuboid_window,
                                float ray_origin_x, float ray_origin_y,
                                float ray_origin_z, float ray_direction_x,
                                float ray_direction_y, float ray_direction_z)
{
  delegate_->HandleRayEnter(ray, serial, cuboid_window, ray_origin_x,
                            ray_origin_y, ray_origin_z, ray_direction_x,
                            ray_direction_y, ray_direction_z);
}

template <class T>
void ZWindow<T>::HandleRayMotion(struct z11_ray *ray, uint32_t time,
                                 float ray_origin_x, float ray_origin_y,
                                 float ray_origin_z, float ray_direction_x,
                                 float ray_direction_y, float ray_direction_z)
{
  delegate_->HandleRayMotion(ray, time, ray_origin_x, ray_origin_y,
                             ray_origin_z, ray_direction_x, ray_direction_y,
                             ray_direction_z);
}

template <class T>
void ZWindow<T>::HandleRayLeave(struct z11_ray *ray, uint32_t serial,
                                struct z11_cuboid_window *cuboid_window)
{
  delegate_->HandleRayLeave(ray, serial, cuboid_window);
}

template <class T>
void ZWindow<T>::HandleRayButton(struct z11_ray *ray, uint32_t serial,
                                 uint32_t time, uint32_t button, uint32_t state)
{
  delegate_->HandleRayButton(ray, serial, time, button, state);
}

template <class T>
void ZWindow<T>::HandleKeyboardKeymap(struct z11_keyboard *keyboard,
                                      uint32_t format, int fd, uint32_t size)
{
  delegate_->HandleKeyboardKeymap(keyboard, format, fd, size);
}

template <class T>
void ZWindow<T>::HandleKeyboardEnter(struct z11_keyboard *keyboard,
                                     uint32_t serial,
                                     struct z11_cuboid_window *cuboid_window,
                                     struct wl_array *keys)
{
  delegate_->HandleKeyboardEnter(keyboard, serial, cuboid_window, keys);
}

template <class T>
void ZWindow<T>::HandleKeyboardLeave(struct z11_keyboard *keyboard,
                                     uint32_t serial,
                                     struct z11_cuboid_window *cuboid_window)
{
  delegate_->HandleKeyboardLeave(keyboard, serial, cuboid_window);
}

template <class T>
void ZWindow<T>::HandleKeyboardKey(struct z11_keyboard *keyboard,
                                   uint32_t serial, uint32_t time, uint32_t key,
                                   uint32_t state)
{
  delegate_->HandleKeyboardKey(keyboard, serial, time, key, state);
}

template <class T>
void ZWindow<T>::HandleKeyboardModifiers(struct z11_keyboard *keyboard,
                                         uint32_t serial,
                                         uint32_t mods_depressed,
                                         uint32_t mods_latached,
                                         uint32_t mods_locked, uint32_t group)
{
  delegate_->HandleKeyboardModifiers(keyboard, serial, mods_depressed,
                                     mods_latached, mods_locked, group);
}

template <class T>
void ZWindow<T>::ShmFormat(struct wl_shm *wl_shm, uint32_t format)
{
  (void)wl_shm;
  (void)format;
}

template <class T>
int ZWindow<T>::CreateSharedFD(off_t size)
{
  const char *name = "z11-simple-box";

  int fd = memfd_create(name, MFD_CLOEXEC | MFD_ALLOW_SEALING);
  if (fd < 0) return fd;
  unlink(name);

  if (ftruncate(fd, size) < 0) {
    close(fd);
    return -1;
  }

  return fd;
}

template <class T>
inline struct wl_display *ZWindow<T>::display()
{
  return display_;
}

template <class T>
inline struct wl_registry *ZWindow<T>::registry()
{
  return registry_;
}

template <class T>
inline struct z11_compositor *ZWindow<T>::compositor()
{
  return compositor_;
}

template <class T>
inline struct wl_shm *ZWindow<T>::shm()
{
  return shm_;
}

template <class T>
inline struct z11_shell *ZWindow<T>::shell()
{
  return shell_;
}

template <class T>
inline struct z11_opengl *ZWindow<T>::gl()
{
  return gl_;
}

template <class T>
inline struct z11_seat *ZWindow<T>::seat()
{
  return seat_;
}

template <class T>
inline struct z11_opengl_render_component_manager *
ZWindow<T>::render_component_manager()
{
  return render_component_manager_;
}

template <class T>
inline struct z11_data_device_manager *ZWindow<T>::data_device_manager()
{
  return data_device_manager_;
}

#endif  //  Z11_CLIENT_Z_WINDOW_H
