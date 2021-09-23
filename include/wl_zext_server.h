#ifndef WL_ZEXT_SERVER_H
#define WL_ZEXT_SERVER_H

#include <wayland-server-core.h>

#define wl_zext_shm_raw_buffer wl_shm_buffer
#define wl_zext_shm_raw_buffer_get wl_shm_buffer_get
#define wl_zext_shm_raw_buffer_begin_access wl_shm_buffer_begin_access
#define wl_zext_shm_raw_buffer_end_access wl_shm_buffer_end_access
#define wl_zext_shm_raw_buffer_get_data wl_shm_buffer_get_data
#define wl_zext_shm_raw_buffer_ref_pool wl_shm_buffer_ref_pool

int32_t wl_zext_shm_raw_buffer_get_size(struct wl_zext_shm_raw_buffer *buffer)
{
  int32_t stride = wl_shm_buffer_get_stride(buffer);
  int32_t height = wl_shm_buffer_get_height(buffer);
  return stride * height;
}

#endif  //  WL_ZEXT_SERVER_H
