#include <GL/glew.h>
#include <wayland-server.h>

#include "internal.h"

struct zazen_gl_shader_program {
  GLuint id;
  struct wl_signal destroy_signal;
};

void zazen_gl_shader_program_destroy(struct zazen_gl_shader_program* shader_program);

static void zazen_gl_shader_program_handle_destroy(struct wl_resource* resource)
{
  struct zazen_gl_shader_program* shader_program = wl_resource_get_user_data(resource);

  zazen_gl_shader_program_destroy(shader_program);
}

static void zazen_gl_shader_program_protocol_destroy(struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static const struct z11_gl_shader_program_interface zazen_gl_shader_program_interface = {
    .destroy = zazen_gl_shader_program_protocol_destroy,
};

/**
 * Be sure that each source string is null termintaed
 * @param vertex_shader_source null terminated vertex shader source string
 * @param fragment_shader_source null terminated fragment shader source string
 * @return 0 when compilation fails
 */
static uint32_t z11_gl_shader_program_compile_shaders(GLuint program_id, const char* vertex_shader_source,
                                                      const char* fragment_shader_source)
{
  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
  glCompileShader(vertex_shader);

  GLint vertex_shader_compiled = GL_FALSE;
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_shader_compiled);
  if (vertex_shader_compiled != GL_TRUE) {
    glDeleteShader(vertex_shader);
    return 0;
  }
  glAttachShader(program_id, vertex_shader);
  glDeleteShader(vertex_shader);

  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
  glCompileShader(fragment_shader);

  GLint fragment_shader_compiled = GL_FALSE;
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_shader_compiled);
  if (fragment_shader_compiled != GL_TRUE) {
    glDeleteShader(fragment_shader);
    return 0;
  }
  glAttachShader(program_id, fragment_shader);
  glDeleteShader(fragment_shader);

  glLinkProgram(program_id);

  GLint program_success = GL_TRUE;
  glGetProgramiv(program_id, GL_LINK_STATUS, &program_success);
  if (program_success != GL_TRUE) {
    return 0;
  }

  glUseProgram(program_id);
  glUseProgram(0);

  return program_id;
}

struct zazen_gl_shader_program* zazen_gl_shader_program_create(struct wl_client* client, uint32_t id,
                                                               const char* vertex_shader_source,
                                                               const char* fragment_shader_source)
{
  struct zazen_gl_shader_program* shader_program;
  struct wl_resource* resource;

  shader_program = zalloc(sizeof *shader_program);
  if (shader_program == NULL) {
    wl_client_post_no_memory(client);
    goto no_mem_shader_program;
  }

  resource = wl_resource_create(client, &z11_gl_shader_program_interface, 1, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    goto no_mem_resource;
  }

  wl_resource_set_implementation(resource, &zazen_gl_shader_program_interface, shader_program,
                                 zazen_gl_shader_program_handle_destroy);

  wl_signal_init(&shader_program->destroy_signal);

  shader_program->id = glCreateProgram();
  if (shader_program->id == 0 || z11_gl_shader_program_compile_shaders(
                                     shader_program->id, vertex_shader_source, fragment_shader_source) == 0) {
    wl_resource_post_error(resource, Z11_GL_SHADER_PROGRAM_ERROR_COMPILATION_ERROR,
                           "failed to compile shaders");
    wl_resource_destroy(resource);
    return NULL;
  }

  return shader_program;

no_mem_resource:
  free(shader_program);

no_mem_shader_program:
  return NULL;
}

void zazen_gl_shader_program_destroy(struct zazen_gl_shader_program* shader_program)
{
  wl_signal_emit(&shader_program->destroy_signal, shader_program);
  glDeleteProgram(shader_program->id);
  free(shader_program);
}

struct wl_signal* zazen_gl_shader_program_get_destroy_signal(struct zazen_gl_shader_program* shader_program)
{
  return &shader_program->destroy_signal;
}

GLuint zazen_gl_shader_program_get_id(struct zazen_gl_shader_program* shader_program)
{
  return shader_program->id;
}
