#define _GNU_SOURCE 1

#include "opengl_shader_program.h"

#include <string.h>
#include <wayland-server.h>

#include "util.h"
#include "z11-opengl-server-protocol.h"
#include "z11-server-protocol.h"

static void zazen_opengl_shader_program_destroy(
    struct zazen_opengl_shader_program* shader_program);

static void zazen_opengl_shader_program_handle_destroy(
    struct wl_resource* resource)
{
  struct zazen_opengl_shader_program* shader_program =
      wl_resource_get_user_data(resource);

  zazen_opengl_shader_program_destroy(shader_program);
}

static void zazen_opengl_shader_program_protocol_destroy(
    struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static const struct z11_opengl_shader_program_interface
    zazen_opengl_shader_program_interface = {
        .destroy = zazen_opengl_shader_program_protocol_destroy,
};

struct zazen_opengl_shader_program* zazen_opengl_shader_program_create(
    struct wl_client* client, uint32_t id, const char* vertex_shader_source,
    const char* fragment_shader_source)
{
  struct zazen_opengl_shader_program* shader_program;
  struct wl_resource* resource;

  shader_program = zalloc(sizeof *shader_program);
  if (shader_program == NULL) {
    wl_client_post_no_memory(client);
    goto no_mem_shader_program;
  }

  resource =
      wl_resource_create(client, &z11_opengl_shader_program_interface, 1, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    goto no_mem_resource;
  }

  shader_program->vertex_shader_source = strdup(vertex_shader_source);
  if (shader_program->vertex_shader_source == NULL)
    goto no_mem_vertex_shader_source;
  shader_program->fragment_shader_source = strdup(fragment_shader_source);
  if (shader_program->fragment_shader_source == NULL)
    goto no_mem_fragment_shader_source;

  wl_resource_set_implementation(
      resource, &zazen_opengl_shader_program_interface, shader_program,
      zazen_opengl_shader_program_handle_destroy);

  wl_signal_init(&shader_program->destroy_signal);
  wl_signal_init(&shader_program->state_change_signal);

  return shader_program;

no_mem_fragment_shader_source:
  free(shader_program->vertex_shader_source);

no_mem_vertex_shader_source:
  free(resource);

no_mem_resource:
  free(shader_program);

no_mem_shader_program:
  return NULL;
}

static void zazen_opengl_shader_program_destroy(
    struct zazen_opengl_shader_program* shader_program)
{
  wl_signal_emit(&shader_program->destroy_signal, shader_program);
  free(shader_program->vertex_shader_source);
  free(shader_program->fragment_shader_source);
  free(shader_program);
}
