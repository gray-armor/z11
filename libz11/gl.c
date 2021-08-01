#include <wayland-server.h>

#include "internal.h"
#include "z11-server-protocol.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
struct z_gl {};
#pragma GCC diagnostic pop

static void z_gl_protocol_create_vertex_buffer(struct wl_client* client, struct wl_resource* resource,
                                               uint32_t id)
{
  UNUSED(resource);
  struct z_gl_vertex_buffer* gl_vertex_buffer;

  gl_vertex_buffer = z_gl_vertex_buffer_create(client, id);
  if (gl_vertex_buffer == NULL) {
    // TODO: error log
  }
}

static void z_gl_protocol_create_shader_program(struct wl_client* client, struct wl_resource* resource,
                                                uint32_t id, const char* vertex_shader_source,
                                                const char* fragment_shader_source)
{
  UNUSED(resource);
  struct z_gl_shader_program* shader_program;

  shader_program = z_gl_shader_program_create(client, id, vertex_shader_source, fragment_shader_source);
  if (shader_program == NULL) {
    // TODO: error log
  }
}

static const struct z11_gl_interface z_gl_interface = {
    .create_vertex_buffer = z_gl_protocol_create_vertex_buffer,
    .create_shader_program = z_gl_protocol_create_shader_program,
};

static void z_gl_bind(struct wl_client* client, void* data, uint32_t version, uint32_t id)
{
  struct z_gl* gl = data;
  struct wl_resource* resource;

  resource = wl_resource_create(client, &z11_gl_interface, version, id);
  if (resource == NULL) goto no_mem_resource;

  wl_resource_set_implementation(resource, &z_gl_interface, gl, NULL);

  return;

no_mem_resource:
  wl_client_post_no_memory(client);
}

struct z_gl* z_gl_create(struct wl_display* display)
{
  struct z_gl* gl;

  gl = zalloc(sizeof *gl);
  if (gl == NULL) goto fail;

  if (wl_global_create(display, &z11_gl_interface, 1, gl, z_gl_bind) == NULL) goto fail;

  return gl;

fail:
  return NULL;
}
