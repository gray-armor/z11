#ifndef WAYLAND_SERVER_CORE_HPP
#define WAYLAND_SERVER_CORE_HPP

#include <wayland-server-protocol.h>

namespace wl_cpp {

template <class T>
struct wl_global *global_create(struct wl_display *display, const wl_interface *interface, int version,
                                T *instance)
{
  auto bind = [](struct wl_client *client, void *data, uint32_t version, uint32_t id) {
    T *instance = static_cast<T *>(data);
    instance->Bind(client, version, id);
  };
  return wl_global_create(display, interface, version, instance, bind);
}

}  // namespace wl_cpp

#endif  // WAYLAND_SERVER_CORE_HPP
