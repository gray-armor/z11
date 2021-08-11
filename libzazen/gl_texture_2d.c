#include <GL/glew.h>

#include "internal.h"
#include "z11-server-protocol.h"

struct zazen_gl_texture_2d {
  GLuint id;
  struct wl_signal destroy_signal;
};

static void zazen_gl_texture_2d_destroy(struct zazen_gl_texture_2d* texture);

static void zazen_gl_texture_2d_handle_destroy(struct wl_resource* resource)
{
  struct zazen_gl_texture_2d* texture = wl_resource_get_user_data(resource);

  zazen_gl_texture_2d_destroy(texture);
}

static void zazen_gl_texture_2d_protocol_destroy(struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void zazen_gl_texture_2d_protocol_set_image(struct wl_client* client, struct wl_resource* resource,
                                                   struct wl_resource* raw_buffer,
                                                   enum z11_gl_texture_2d_format format, int32_t width,
                                                   int32_t height)
{
  UNUSED(client);
  struct zazen_gl_texture_2d* texture;
  struct wl_shm_raw_buffer* shm_raw_buffer;
  int32_t data_size;
  void* data;

  texture = wl_resource_get_user_data(resource);
  shm_raw_buffer = wl_shm_raw_buffer_get(raw_buffer);
  data_size = wl_shm_raw_buffer_get_size(shm_raw_buffer);
  data = wl_shm_raw_buffer_get_data(shm_raw_buffer);

  glBindTexture(GL_TEXTURE_2D, texture->id);
  if (format == Z11_GL_TEXTURE_2D_FORMAT_ARGB8888 && data_size <= width * height * 4) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, data);
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
}

static const struct z11_gl_texture_2d_interface zazen_gl_texture_2d_interface = {
    .destroy = zazen_gl_texture_2d_protocol_destroy,
    .set_image = zazen_gl_texture_2d_protocol_set_image,
};

struct zazen_gl_texture_2d* zazen_gl_texture_2d_create(struct wl_client* client, uint32_t id)
{
  struct zazen_gl_texture_2d* texture;
  struct wl_resource* resource;

  texture = zalloc(sizeof *texture);
  if (texture == NULL) goto no_mem_texture;

  resource = wl_resource_create(client, &z11_gl_texture_2d_interface, 1, id);
  if (resource == NULL) goto no_mem_resource;

  wl_resource_set_implementation(resource, &zazen_gl_texture_2d_interface, texture,
                                 zazen_gl_texture_2d_handle_destroy);

  wl_signal_init(&texture->destroy_signal);

  glGenTextures(1, &texture->id);

  return texture;

no_mem_resource:
  free(texture);

no_mem_texture:
  wl_client_post_no_memory(client);
  return NULL;
}

static void zazen_gl_texture_2d_destroy(struct zazen_gl_texture_2d* texture)
{
  wl_signal_emit(&texture->destroy_signal, texture);
  glDeleteTextures(1, &texture->id);
  free(texture);
}

struct wl_signal* zazen_gl_texture_2d_get_destroy_signal(struct zazen_gl_texture_2d* texture)
{
  return &texture->destroy_signal;
}

GLuint zazen_gl_texture_2d_get_id(struct zazen_gl_texture_2d* texture) { return texture->id; }
