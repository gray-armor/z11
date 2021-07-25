#include "shader.h"

#include <GL/glew.h>
#include <stdio.h>

bool Shader::Init(const char *shader_name, const char *vertex_shader, const char *fragment_shader)
{
  id_ = glCreateProgram();

  // compile vertex shader
  GLuint scene_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(scene_vertex_shader, 1, &vertex_shader, NULL);
  glCompileShader(scene_vertex_shader);

  GLint vertex_shader_compiled = GL_FALSE;
  glGetShaderiv(scene_vertex_shader, GL_COMPILE_STATUS, &vertex_shader_compiled);
  if (vertex_shader_compiled != GL_TRUE) {
    fprintf(stderr, "%s - Unable to compile vertex shader %d!\n", shader_name, scene_vertex_shader);
    glDeleteProgram(id_);
    glDeleteShader(scene_vertex_shader);
    return false;
  }
  glAttachShader(id_, scene_vertex_shader);
  glDeleteShader(scene_vertex_shader);

  // compile fragmen shader
  GLuint scene_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(scene_fragment_shader, 1, &fragment_shader, NULL);
  glCompileShader(scene_fragment_shader);

  GLint fragment_shader_compiled = GL_FALSE;
  glGetShaderiv(scene_fragment_shader, GL_COMPILE_STATUS, &fragment_shader_compiled);
  if (fragment_shader_compiled != GL_TRUE) {
    fprintf(stderr, "%s - Unable to compile fragment shader %d!\n", shader_name, scene_fragment_shader);
    glDeleteProgram(id_);
    glDeleteShader(scene_fragment_shader);
    return false;
  }

  glAttachShader(id_, scene_fragment_shader);
  glDeleteShader(scene_fragment_shader);

  // check
  glLinkProgram(id_);

  GLint program_success = GL_TRUE;
  glGetProgramiv(id_, GL_LINK_STATUS, &program_success);
  if (program_success != GL_TRUE) {
    fprintf(stderr, "%s - Error linking program %d!\n", shader_name, id_);
    glDeleteProgram(id_);
    return false;
  }

  glUseProgram(id_);
  glUseProgram(0);

  return true;
}
