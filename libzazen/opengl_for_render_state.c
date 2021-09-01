#include "opengl_for_render_state.h"

#include <GL/glew.h>
#include <libzazen.h>
#include <wayland-server.h>

#include "util.h"

struct zazen_opengl_render_item* zazen_opengl_render_item_create(
    struct zazen_opengl_render_component_manager* manager)
{
  struct zazen_opengl_render_item* render_item;
  render_item = zalloc(sizeof *render_item);
  if (render_item == NULL) {
    zazen_log("Unable to create render item");
    goto out;
  }

  render_item->manager = manager;

  render_item->state_changed = true;

  render_item->vertex_buffer_data = NULL;
  render_item->texture_2d_data = NULL;

  render_item->vertex_shader_source = NULL;
  render_item->fragment_shader_source = NULL;

  wl_list_init(&render_item->back_state.link);

  return render_item;

out:
  return NULL;
}

static bool commit_shader_program(struct zazen_opengl_render_item* render_item);
static void commit_texture_2d(struct zazen_opengl_render_item* render_item);
static void commit_vertex_buffer(struct zazen_opengl_render_item* render_item);

void zazen_opengl_render_item_commit(struct zazen_opengl_render_item* render_item)
{
  wl_list_remove(&render_item->back_state.link);
  wl_list_init(&render_item->back_state.link);

  commit_texture_2d(render_item);
  if (commit_shader_program(render_item) == false) {
    // TODO: Error handle
    return;
  }
  commit_vertex_buffer(render_item);

  glDeleteVertexArrays(1, &render_item->back_state.vertex_array_id);
  render_item->back_state.vertex_array_id = 0;

  glGenVertexArrays(1, &render_item->back_state.vertex_array_id);

  glBindVertexArray(render_item->back_state.vertex_array_id);
  glBindBuffer(GL_ARRAY_BUFFER, render_item->back_state.vertex_buffer_id);

  glEnableVertexAttribArray(render_item->vertex_location);
  // TODO: handle multiple vertex input attribute
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
  glVertexAttribPointer(render_item->vertex_location, render_item->vertex_size, render_item->vertex_type,
                        GL_FALSE, render_item->back_state.vertex_stride, (void*)render_item->vertex_offset);
#pragma GCC diagnostic pop
  glBindVertexArray(0);

  // Line* line = (Line*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
  // zazen_log("b: (%f, %f, %f)\n", line->begin.x, line->begin.y, line->begin.z);
  // zazen_log("e: (%f, %f, %f)\n", line->end.x, line->end.y, line->end.z);

  wl_list_insert(&render_item->manager->render_component_back_state_list, &render_item->back_state.link);
}

static bool commit_shader_program(struct zazen_opengl_render_item* render_item)
{
  glDeleteProgram(render_item->back_state.shader_program_id);
  render_item->back_state.shader_program_id = 0;

  if (render_item->vertex_shader_source == NULL || render_item->fragment_shader_source == NULL) return true;

  render_item->back_state.shader_program_id = glCreateProgram();

  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, (const char**)&render_item->vertex_shader_source, NULL);
  glCompileShader(vertex_shader);

  GLint vertex_shader_compiled = GL_FALSE;
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_shader_compiled);
  if (vertex_shader_compiled != GL_TRUE) {
    glDeleteShader(vertex_shader);
    // TODO: Error Handling
    goto out;
  }
  glAttachShader(render_item->back_state.shader_program_id, vertex_shader);
  glDeleteShader(vertex_shader);

  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, (const char**)&render_item->fragment_shader_source, NULL);
  glCompileShader(fragment_shader);
  GLint fragment_shader_compiled = GL_FALSE;
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_shader_compiled);
  if (fragment_shader_compiled != GL_TRUE) {
    glDeleteShader(fragment_shader);
    // TODO: Error Handling
    goto out;
  }
  glAttachShader(render_item->back_state.shader_program_id, fragment_shader);
  glDeleteShader(fragment_shader);

  glLinkProgram(render_item->back_state.shader_program_id);

  GLint program_success = GL_TRUE;
  glGetProgramiv(render_item->back_state.shader_program_id, GL_LINK_STATUS, &program_success);
  if (program_success != GL_TRUE) {
    // TODO: Error Handling
    goto out;
  }
  glUseProgram(render_item->back_state.shader_program_id);
  glUseProgram(0);

  return true;

out:
  glDeleteProgram(render_item->back_state.shader_program_id);
  render_item->back_state.shader_program_id = 0;
  return false;
}

static void commit_vertex_buffer(struct zazen_opengl_render_item* render_item)
{
  glDeleteBuffers(1, &render_item->back_state.vertex_buffer_id);
  render_item->back_state.vertex_buffer_id = 0;

  if (render_item->vertex_buffer_data == NULL) return;

  glGenBuffers(1, &render_item->back_state.vertex_buffer_id);
  glBindBuffer(GL_ARRAY_BUFFER, render_item->back_state.vertex_buffer_id);
  glBufferData(GL_ARRAY_BUFFER, render_item->back_state.vertex_buffer_size,
               (void*)render_item->vertex_buffer_data, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void commit_texture_2d(struct zazen_opengl_render_item* render_item)
{
  glDeleteTextures(1, &render_item->back_state.texture_2d_id);
  render_item->back_state.texture_2d_id = 0;

  if (render_item->texture_2d_data == NULL) return;

  glGenTextures(1, &render_item->back_state.texture_2d_id);
  glBindTexture(GL_TEXTURE_2D, render_item->back_state.texture_2d_id);
  // TODO: Enable to render texture 2D
  // if (state->format == Z11_OPENGL_TEXTURE_2D_FORMAT_ARGB8888 &&
  //     buffer_size <= state->width * state->height * 4) {
  //   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, state->width, state->height, 0, GL_BGRA,
  //                GL_UNSIGNED_INT_8_8_8_8_REV, render_item->texture_2d_data);
  // }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
}
