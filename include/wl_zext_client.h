#ifndef WL_ZEXT_CLIENT_H
#define WL_ZEXT_CLIENT_H

#include <wayland-client.h>

#define wl_zext_raw_buffer wl_buffer
#define wl_raw_buffer_destroy wl_buffer_destroy

static inline struct wl_zext_raw_buffer *wl_zext_shm_pool_create_raw_buffer(
    struct wl_shm_pool *wl_shm_pool, int32_t offset, int32_t size)
{
  return wl_shm_pool_create_buffer(wl_shm_pool, offset, size, 1, size, 0);
}

// TODO: implement other protocols

#endif  //  WL_ZEXT_CLIENT_H
