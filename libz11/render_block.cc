#include <GL/glew.h>
#include <libz11.h>
#include <wayland-server.h>

#include "z11-server-protocol.h"
#include "z11-server-protocol.hh"

namespace z11 {

RenderBlock* RenderBlock::Create(struct wl_client* client, uint32_t id, Compositor* compositor)
{
  RenderBlock* render_block = new RenderBlock(client, id, compositor);
  if (render_block->created_) return render_block;
  delete render_block;
  return nullptr;
}

RenderBlock::RenderBlock(struct wl_client* client, uint32_t id, Compositor* compositor)
    : created_(false),
      raw_buffer_resource_(nullptr),
      link_(nullptr),
      vertex_array_object_(0),
      vertex_buffer_(0)
{
  struct wl_resource* resource;

  resource = wl_resource_create(client, &z11_render_block_interface, 1, id);
  if (resource == nullptr) goto no_mem_resource;

  z_cpp::z_render_block_resource_bind(resource, this);

  created_ = true;
  link_ = new List<RenderBlock>(this);
  compositor->PushRenderBlock(this->link_);

  glGenVertexArrays(1, &vertex_array_object_);
  glGenBuffers(1, &vertex_buffer_);

  return;

no_mem_resource:
  wl_client_post_no_memory(client);
  return;
}

RenderBlock::~RenderBlock()
{
  if (link_ != nullptr) {
    link_->Remove();
    delete link_;
  }
  glDeleteBuffers(1, &vertex_buffer_);
  glDeleteVertexArrays(1, &vertex_array_object_);
}

int32_t RenderBlock::GetDataSize()
{
  if (raw_buffer_resource_ == nullptr) return 0;
  struct wl_shm_raw_buffer* shm_raw_buffer = wl_shm_raw_buffer_get(raw_buffer_resource_);
  return wl_shm_raw_buffer_get_size(shm_raw_buffer);
}

void* RenderBlock::GetData()
{
  if (raw_buffer_resource_ == nullptr) return nullptr;
  struct wl_shm_raw_buffer* shm_raw_buffer = wl_shm_raw_buffer_get(raw_buffer_resource_);
  return wl_shm_raw_buffer_get_data(shm_raw_buffer);
}

void RenderBlock::Rebind()
{
  glBindVertexArray(vertex_array_object_);
  if (raw_buffer_resource_ == nullptr) goto clean;
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  glBufferData(GL_ARRAY_BUFFER, GetDataSize(), GetData(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

clean:
  glBindVertexArray(0);
  glDisableVertexAttribArray(0);
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
inline void RenderBlock::Attach(struct wl_client* client, struct wl_resource* raw_buffer_resource)
{
  raw_buffer_resource_ = raw_buffer_resource;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
inline void RenderBlock::Commit(struct wl_client* client) { Rebind(); }

}  // namespace z11
