#include <GL/glew.h>
#include <wayland-server.h>

#include "internal.h"
#include "z11-server-protocol.h"

struct z_render_block {
  struct z_compositor* compositor;
  struct wl_list link;
  struct wl_resource* raw_buffer;
  GLuint vertex_array_object;
  GLuint vertex_buffer;
  uint32_t vertex_buffer_size;
};

void z_render_block_destroy(struct z_render_block* render_block);

static void z_render_block_handle_destroy(struct wl_resource* resource)
{
  struct z_render_block* render_block = wl_resource_get_user_data(resource);

  z_render_block_destroy(render_block);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
static void z_render_block_protocol_attach(struct wl_client* client, struct wl_resource* resource,
                                           struct wl_resource* raw_buffer)
{
  struct z_render_block* render_block = wl_resource_get_user_data(resource);

  render_block->raw_buffer = raw_buffer;
}
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
static void z_render_block_protocol_commit(struct wl_client* client, struct wl_resource* resource)
{
  struct z_render_block* render_block = wl_resource_get_user_data(resource);
  if (render_block->raw_buffer == NULL) return;

  struct wl_shm_raw_buffer* shm_raw_buffer = wl_shm_raw_buffer_get(render_block->raw_buffer);
  void* data = wl_shm_raw_buffer_get_data(shm_raw_buffer);
  int32_t size = wl_shm_raw_buffer_get_size(shm_raw_buffer);

  glBindVertexArray(render_block->vertex_array_object);
  glBindBuffer(GL_ARRAY_BUFFER, render_block->vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  render_block->vertex_buffer_size = size;
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glBindVertexArray(0);
  glDisableVertexAttribArray(0);
}
#pragma GCC diagnostic pop

static const struct z11_render_block_interface z_render_block_interface = {
    .attach = z_render_block_protocol_attach,
    .commit = z_render_block_protocol_commit,
};

struct wl_list* z_render_block_get_link(struct z_render_block* render_block) { return &render_block->link; }

void z_render_block_draw(struct z_render_block* render_block)
{
  glBindVertexArray(render_block->vertex_array_object);
  glDrawArrays(GL_LINES, 0, render_block->vertex_buffer_size / (sizeof(float) * 3));
  glBindVertexArray(0);
}

struct z_render_block* z_render_block_from_link(struct wl_list* list)
{
  struct z_render_block* render_block;
  render_block = wl_container_of(list, render_block, link);
  return render_block;
}

struct z_render_block* z_render_block_create(struct wl_client* client, uint32_t id,
                                             struct z_compositor* compositor)
{
  struct z_render_block* render_block;
  struct wl_resource* resource;

  render_block = zalloc(sizeof *render_block);
  if (render_block == NULL) goto no_mem_render_block;

  resource = wl_resource_create(client, &z11_render_block_interface, 1, id);
  if (resource == NULL) goto no_mem_resource;

  render_block->compositor = compositor;
  wl_list_init(&render_block->link);
  render_block->raw_buffer = NULL;
  glGenVertexArrays(1, &render_block->vertex_array_object);
  glGenBuffers(1, &render_block->vertex_buffer);

  wl_resource_set_implementation(resource, &z_render_block_interface, render_block,
                                 z_render_block_handle_destroy);

  z_compositor_append_render_block(compositor, render_block);

  return render_block;

no_mem_resource:
  free(render_block);

no_mem_render_block:
  wl_client_post_no_memory(client);
  return NULL;
}

void z_render_block_destroy(struct z_render_block* render_block)
{
  wl_list_remove(&render_block->link);
  glDeleteBuffers(1, &render_block->vertex_buffer);
  glDeleteVertexArrays(1, &render_block->vertex_array_object);
  free(render_block);
}
