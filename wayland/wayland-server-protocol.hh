#ifndef WAYLAND_SERVER_PROTOCOL_HPP
#define WAYLAND_SERVER_PROTOCOL_HPP

#include <wayland-server-protocol.h>

namespace wl_cpp {

namespace private_ {

template <class T>
static void destroy_instance(struct wl_resource *resource)
{
  T *instance = static_cast<T *>(wl_resource_get_user_data(resource));
  delete instance;
}

template <class T>
static struct wl_compositor_interface compositor_interface = {
    [](struct wl_client *client, struct wl_resource *resource, uint32_t id) {
      T *compositor = static_cast<T *>(wl_resource_get_user_data(resource));
      compositor->CreateSurface(client, id);
    },
    [](struct wl_client *client, struct wl_resource *resource, uint32_t id) {
      T *compositor = static_cast<T *>(wl_resource_get_user_data(resource));
      compositor->CreateRegion(client, id);
    },
};

}  // namespace private_

template <class T>
void compositor_resource_bind(struct wl_resource *resource, T *instance)
{
  wl_resource_set_implementation(resource, &private_::compositor_interface<T>, instance,
                                 private_::destroy_instance<T>);
}

}  // namespace wl_cpp

#endif  // WAYLAND_SERVER_PROTOCOL_HPP
