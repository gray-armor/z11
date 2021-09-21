
#include "opengl_render_component_back_state.h"

#include <GL/glew.h>
#include <cglm/cglm.h>
#include <string.h>
#include <wayland-util.h>

#include "util.h"
#include "z11-opengl-server-protocol.h"

static GLuint get_size_from_attribute_format(
    enum z11_opengl_vertex_input_attribute_format format);
static GLenum get_type_from_attribute_format(
    enum z11_opengl_vertex_input_attribute_format format);
static GLenum get_topology_mode(enum z11_opengl_topology topology);

void zazen_opengl_render_component_back_state_delete_texture_2d(
    struct zazen_opengl_render_component_back_state* back_state)
{
  glDeleteTextures(1, &back_state->texture_2d_id);
  back_state->texture_2d_id = 0;
}

void zazen_opengl_render_component_back_state_generate_texture_2d(
    struct zazen_opengl_render_component_back_state* back_state,
    enum z11_opengl_texture_2d_format format, int32_t width, int32_t height,
    void* data, int32_t buffer_size)
{
  glGenTextures(1, &back_state->texture_2d_id);
  glBindTexture(GL_TEXTURE_2D, back_state->texture_2d_id);
  if (format == Z11_OPENGL_TEXTURE_2D_FORMAT_ARGB8888 &&
      buffer_size <= width * height * 4) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA,
                 GL_UNSIGNED_INT_8_8_8_8_REV, data);
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void zazen_opengl_render_component_back_state_delete_shader_program(
    struct zazen_opengl_render_component_back_state* back_state)
{
  glDeleteProgram(back_state->shader_program_id);
  back_state->shader_program_id = 0;
}

bool zazen_opengl_render_component_back_state_generate_shader_program(
    struct zazen_opengl_render_component_back_state* back_state,
    const char* vertex_shader_source, const char* fragment_shader_source)
{
  back_state->shader_program_id = glCreateProgram();

  GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader_id, 1, (const char**)&vertex_shader_source,
                 NULL);
  glCompileShader(vertex_shader_id);
  GLint vertex_shader_compiled = GL_FALSE;
  glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &vertex_shader_compiled);
  if (vertex_shader_compiled != GL_TRUE) {
    zazen_log("vertex shader compile failed");
    glDeleteShader(vertex_shader_id);
    goto out;
  }
  glAttachShader(back_state->shader_program_id, vertex_shader_id);
  glDeleteShader(vertex_shader_id);

  GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader_id, 1, (const char**)&fragment_shader_source,
                 NULL);
  glCompileShader(fragment_shader_id);
  GLint fragment_shader_compiled = GL_FALSE;
  glGetShaderiv(fragment_shader_id, GL_COMPILE_STATUS,
                &fragment_shader_compiled);
  if (fragment_shader_compiled != GL_TRUE) {
    zazen_log("fragment shader compile failed");
    glDeleteShader(fragment_shader_id);
    goto out;
  }
  glAttachShader(back_state->shader_program_id, fragment_shader_id);
  glDeleteShader(fragment_shader_id);

  glLinkProgram(back_state->shader_program_id);

  GLint program_success = GL_TRUE;
  glGetProgramiv(back_state->shader_program_id, GL_LINK_STATUS,
                 &program_success);
  if (program_success != GL_TRUE) {
    zazen_log("shader program link failed");
    goto out;
  }

  glUseProgram(back_state->shader_program_id);
  glUseProgram(0);

  return true;

out:
  glDeleteProgram(back_state->shader_program_id);
  back_state->shader_program_id = 0;
  return false;
}

void zazen_opengl_render_component_back_state_delete_vertex_buffer(
    struct zazen_opengl_render_component_back_state* back_state)
{
  glDeleteBuffers(1, &back_state->vertex_buffer_id);
  back_state->vertex_buffer_id = 0;
}

void zazen_opengl_render_component_back_state_generate_vertex_buffer(
    struct zazen_opengl_render_component_back_state* back_state,
    int32_t buffer_size, void* data, uint32_t stride)
{
  glGenBuffers(1, &back_state->vertex_buffer_id);
  glBindBuffer(GL_ARRAY_BUFFER, back_state->vertex_buffer_id);
  glBufferData(GL_ARRAY_BUFFER, buffer_size, data, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  back_state->vertex_stride = stride;
  back_state->vertex_buffer_size = buffer_size;
}

void zazen_opengl_render_component_back_state_set_topology_mode(
    struct zazen_opengl_render_component_back_state* back_state,
    enum z11_opengl_topology topology)
{
  back_state->topology_mode = get_topology_mode(topology);
}

void zazen_opengl_render_component_back_state_set_model_view(
    struct zazen_opengl_render_component_back_state* back_state,
    mat4 model_matrix)
{
  memcpy(back_state->model_matrix, model_matrix, sizeof(float) * 16);
}

void zazen_opengl_render_component_back_state_delete_vertex_array(
    struct zazen_opengl_render_component_back_state* back_state)
{
  glDeleteVertexArrays(1, &back_state->vertex_array_id);
  back_state->vertex_array_id = 0;
}

void zazen_opengl_render_component_back_state_generate_vertex_array(
    struct zazen_opengl_render_component_back_state* back_state,
    struct wl_array* vertex_input_attributes)
{
  glGenVertexArrays(1, &back_state->vertex_array_id);

  glBindVertexArray(back_state->vertex_array_id);
  glBindBuffer(GL_ARRAY_BUFFER, back_state->vertex_buffer_id);

  struct zazen_opengl_render_component_back_state_vertex_input_attribute*
      vertex_input_attribute;
  wl_array_for_each(vertex_input_attribute, vertex_input_attributes)
  {
    GLint size = get_size_from_attribute_format(vertex_input_attribute->format);
    GLenum type =
        get_type_from_attribute_format(vertex_input_attribute->format);
    glEnableVertexAttribArray(vertex_input_attribute->location);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
    glVertexAttribPointer(vertex_input_attribute->location, size, type,
                          GL_FALSE, back_state->vertex_stride,
                          (void*)vertex_input_attribute->offset);
#pragma GCC diagnostic pop
  }
  glBindVertexArray(0);
}

void zazen_opengl_render_component_back_state_destroy(
    struct zazen_opengl_render_component_back_state* back_state)
{
  zazen_opengl_render_component_back_state_delete_texture_2d(back_state);
  zazen_opengl_render_component_back_state_delete_shader_program(back_state);
  zazen_opengl_render_component_back_state_delete_vertex_buffer(back_state);
  zazen_opengl_render_component_back_state_delete_vertex_array(back_state);
}

static GLuint get_size_from_attribute_format(
    enum z11_opengl_vertex_input_attribute_format format)
{
  switch (format) {
    case Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_SCALAR:
      return 1;
    case Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR2:
      return 2;
    case Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR3:
      return 3;
    case Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR4:
      return 4;
    default:
      return 0;
  }
}

static GLenum get_type_from_attribute_format(
    enum z11_opengl_vertex_input_attribute_format format)
{
  switch (format) {
    case Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_SCALAR:
    case Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR2:
    case Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR3:
    case Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR4:
      return GL_FLOAT;
    default:
      return GL_FLOAT;
  }
}

static GLenum get_topology_mode(enum z11_opengl_topology topology)
{
  switch (topology) {
    case Z11_OPENGL_TOPOLOGY_POINTS:
      return GL_POINTS;
    case Z11_OPENGL_TOPOLOGY_LINES:
      return GL_LINES;
    case Z11_OPENGL_TOPOLOGY_LINE_STRIP:
      return GL_LINE_STRIP;
    case Z11_OPENGL_TOPOLOGY_TRIANGLES:
      return GL_TRIANGLES;
    case Z11_OPENGL_TOPOLOGY_TRIANGLE_STRIP:
      return GL_TRIANGLE_STRIP;
    case Z11_OPENGL_TOPOLOGY_TRIANGLE_FAN:
      return GL_TRIANGLE_FAN;
    default:
      return GL_LINES;
  }
}
