#include "opengl.h"

#include <wayland-server.h>

#include "opengl_shader_program.h"
#include "opengl_texture_2d.h"
#include "opengl_vertex_buffer.h"
#include "util.h"
#include "z11-opengl-server-protocol.h"

static void zazen_opengl_protocol_create_vertex_buffer(
    struct wl_client *client, struct wl_resource *resource, uint32_t id)
{
  UNUSED(resource);
  struct zazen_opengl_vertex_buffer *vertex_buffer;

  vertex_buffer = zazen_opengl_vertex_buffer_create(client, id);
  if (vertex_buffer == NULL) {
    zazen_log("Failed to create a vertex buffer\n");
  }
}

static void zazen_opengl_protocol_create_shader_program(
    struct wl_client *client, struct wl_resource *resource, uint32_t id,
    const char *vertex_shader_source, const char *fragment_shader_source)
{
  UNUSED(resource);
  struct zazen_opengl_shader_program *shader_program;

  shader_program = zazen_opengl_shader_program_create(
      client, id, vertex_shader_source, fragment_shader_source);
  if (shader_program == NULL) {
    zazen_log("Failed to create a shader program\n");
  }
}

static void zazen_opengl_protocol_create_texture_2d(
    struct wl_client *client, struct wl_resource *resource, uint32_t id)
{
  UNUSED(resource);
  struct zazen_opengl_texture_2d *texture_2d;

  texture_2d = zazen_opengl_texture_2d_create(client, id);
  if (texture_2d == NULL) {
    zazen_log("Failed to create a texture 2d\n");
  }
}

static const struct z11_opengl_interface zazen_opengl_interface = {
    .create_vertex_buffer = zazen_opengl_protocol_create_vertex_buffer,
    .create_shader_program = zazen_opengl_protocol_create_shader_program,
    .create_texture_2d = zazen_opengl_protocol_create_texture_2d,
};

static void zazen_opengl_bind(struct wl_client *client, void *data,
                              uint32_t version, uint32_t id)
{
  struct zazen_opengl *opengl = data;
  struct wl_resource *resource;

  resource = wl_resource_create(client, &z11_opengl_interface, version, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    return;
  }

  wl_resource_set_implementation(resource, &zazen_opengl_interface, opengl,
                                 NULL);
}

struct zazen_opengl *zazen_opengl_create(struct wl_display *display)
{
  struct zazen_opengl *opengl;

  opengl = zalloc(sizeof *opengl);
  if (opengl == NULL) goto out;

  if (wl_global_create(display, &z11_opengl_interface, 1, opengl,
                       zazen_opengl_bind) == NULL)
    goto out;

  return opengl;

out:
  return NULL;
}
