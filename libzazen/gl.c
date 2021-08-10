#include <wayland-server.h>

#include "internal.h"
#include "z11-server-protocol.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
struct zazen_gl {};
#pragma GCC diagnostic pop

static void zazen_gl_protocol_create_vertex_buffer(struct wl_client* client, struct wl_resource* resource,
                                                   uint32_t id)
{
  UNUSED(resource);
  struct zazen_gl_vertex_buffer* gl_vertex_buffer;

  gl_vertex_buffer = zazen_gl_vertex_buffer_create(client, id);
  if (gl_vertex_buffer == NULL) {
    // TODO: error log
  }
}

static void zazen_gl_protocol_create_shader_program(struct wl_client* client, struct wl_resource* resource,
                                                    uint32_t id, const char* vertex_shader_source,
                                                    const char* fragment_shader_source)
{
  UNUSED(resource);
  struct zazen_gl_shader_program* shader_program;

  shader_program = zazen_gl_shader_program_create(client, id, vertex_shader_source, fragment_shader_source);
  if (shader_program == NULL) {
    // TODO: error log
  }
}

static void zazen_gl_protocol_create_texture_2d(struct wl_client* client, struct wl_resource* resource,
                                                uint32_t id)
{
  UNUSED(resource);
  struct zazen_gl_texture_2d* texture = zazen_gl_texture_2d_create(client, id);
  if (texture == NULL) {
    // TODO: error log
  }
}

static const struct z11_gl_interface zazen_gl_interface = {
    .create_vertex_buffer = zazen_gl_protocol_create_vertex_buffer,
    .create_shader_program = zazen_gl_protocol_create_shader_program,
    .create_texture_2d = zazen_gl_protocol_create_texture_2d,
};

static void zazen_gl_bind(struct wl_client* client, void* data, uint32_t version, uint32_t id)
{
  struct zazen_gl* gl = data;
  struct wl_resource* resource;

  resource = wl_resource_create(client, &z11_gl_interface, version, id);
  if (resource == NULL) goto no_mem_resource;

  wl_resource_set_implementation(resource, &zazen_gl_interface, gl, NULL);

  return;

no_mem_resource:
  wl_client_post_no_memory(client);
}

struct zazen_gl* zazen_gl_create(struct wl_display* display)
{
  struct zazen_gl* gl;

  gl = zalloc(sizeof *gl);
  if (gl == NULL) goto fail;

  if (wl_global_create(display, &z11_gl_interface, 1, gl, zazen_gl_bind) == NULL) goto fail;

  return gl;

fail:
  return NULL;
}
