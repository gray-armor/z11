#ifndef Z11_SERVER_PROTOCOL_HPP
#define Z11_SERVER_PROTOCOL_HPP

#include <wayland-server-protocol.h>

#include "wayland-server-protocol.hh"
#include "z11-server-protocol.h"

namespace z_cpp {

namespace private_ {

template <class T>
static void destroy_instance(struct wl_resource *resource)
{
  T *instance = static_cast<T *>(wl_resource_get_user_data(resource));
  delete instance;
}

template <class T>
static const struct z11_compositor_interface compositor_interface = {
    [](struct wl_client *client, struct wl_resource *resource, uint32_t id) {
      T *z_compositor = static_cast<T *>(wl_resource_get_user_data(resource));
      z_compositor->CreateRenderBlock(client, id);
    },
};

#pragma GCC diagnostic ignored "-Wunused-parameter"
template <class T>
static const struct z11_render_block_interface render_block_interface = {
    [](struct wl_client *client, struct wl_resource *resource, struct wl_resource *raw_buffer_resource) {
      T *render_block = static_cast<T *>(wl_resource_get_user_data(resource));
      render_block->Attach(client, raw_buffer_resource);
    },
    [](struct wl_client *client, struct wl_resource *resource) {
      T *render_block = static_cast<T *>(wl_resource_get_user_data(resource));
      render_block->Commit(client);
    },
};

}  // namespace private_

template <class T>
void z_compositor_resource_bind(struct wl_resource *resource, T *instance)
{
  wl_resource_set_implementation(resource, &private_::compositor_interface<T>, instance,
                                 private_::destroy_instance<T>);
}

template <class T>
void z_compositor_resource_bind_no_destroy(struct wl_resource *resource, T *instance)
{
  wl_resource_set_implementation(resource, &private_::compositor_interface<T>, instance, nullptr);
}

template <class T>
void z_render_block_resource_bind(struct wl_resource *resource, T *instance)
{
  wl_resource_set_implementation(resource, &private_::render_block_interface<T>, instance,
                                 private_::destroy_instance<T>);
}

}  // namespace z_cpp

#endif  // Z11_SERVER_PROTOCOL_HPP
