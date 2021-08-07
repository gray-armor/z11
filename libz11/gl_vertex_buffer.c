#include <GL/glew.h>

#include "internal.h"
#include "z11-server-protocol.h"

void z_gl_vertex_buffer_destroy(struct z_gl_vertex_buffer *vertex_buffer);

static void z_gl_vertex_buffer_handle_destroy(struct wl_resource *resource)
{
  struct z_gl_vertex_buffer *vertex_buffer = wl_resource_get_user_data(resource);

  z_gl_vertex_buffer_destroy(vertex_buffer);
}

static void z_gl_vertex_buffer_protocol_destroy(struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void z_gl_vertex_buffer_protocol_allocate(struct wl_client *client, struct wl_resource *resource,
                                                 int32_t size, struct wl_resource *raw_buffer)
{
  UNUSED(client);
  struct z_gl_vertex_buffer *vertex_buffer = wl_resource_get_user_data(resource);

  if (raw_buffer == NULL) {
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer->id);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  } else {
    struct wl_shm_raw_buffer *shm_raw_buffer = wl_shm_raw_buffer_get(raw_buffer);
    void *data = wl_shm_raw_buffer_get_data(shm_raw_buffer);
    int32_t data_size = wl_shm_raw_buffer_get_size(shm_raw_buffer);
    if (data_size < size) {
      // TODO: Error handling
      return;
    }
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer->id);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
  vertex_buffer->size = size;
}

static const struct z11_gl_vertex_buffer_interface z_gl_vertex_buffer_interface = {
    .destroy = z_gl_vertex_buffer_protocol_destroy,
    .allocate = z_gl_vertex_buffer_protocol_allocate,
};

struct z_gl_vertex_buffer *z_gl_vertex_buffer_create(struct wl_client *client, uint32_t id)
{
  struct z_gl_vertex_buffer *vertex_buffer;
  struct wl_resource *resource;

  vertex_buffer = zalloc(sizeof *vertex_buffer);
  if (vertex_buffer == NULL) goto fail;

  resource = wl_resource_create(client, &z11_gl_vertex_buffer_interface, 1, id);
  if (resource == NULL) goto no_mem_resource;

  glGenBuffers(1, &vertex_buffer->id);
  wl_signal_init(&vertex_buffer->destroy_signal);

  wl_resource_set_implementation(resource, &z_gl_vertex_buffer_interface, vertex_buffer,
                                 z_gl_vertex_buffer_handle_destroy);

  return vertex_buffer;

no_mem_resource:
  free(vertex_buffer);

fail:
  wl_client_post_no_memory(client);
  return NULL;
}

void z_gl_vertex_buffer_destroy(struct z_gl_vertex_buffer *vertex_buffer)
{
  wl_signal_emit(&vertex_buffer->destroy_signal, vertex_buffer);
  glDeleteBuffers(1, &vertex_buffer->id);
  free(vertex_buffer);
}
