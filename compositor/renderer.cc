#include "renderer.h"

#include <libz11.h>

bool Renderer::Init()
{
  if (default_shader_.Init(  //
          "Scene",

          // Vertex Shader
          "#version 410\n"
          "uniform mat4 matrix;\n"
          "layout(location = 0) in vec4 position;\n"
          "layout(location = 1) in vec2 v2UVcoordsIn;\n"
          "layout(location = 2) in vec3 v3NormalIn;\n"
          "out vec2 v2UVcoords;\n"
          "void main()\n"
          "{\n"
          "  v2UVcoords = v2UVcoordsIn;\n"
          "  gl_Position = matrix * position;\n"
          "}\n",

          // Fragment Shader
          "#version 410 core\n"
          "in vec2 v2UVcoords;\n"
          "out vec4 outputColor;\n"
          "void main()\n"
          "{\n"
          "  outputColor = vec4(1.0, 0.0, 0.0, 1.0);"
          "}\n") == false)
    return false;
  default_shader_matrix_location_ = glGetUniformLocation(default_shader_.id(), "matrix");

  return true;
}

void Renderer::Render(Eye *eye, z11::List<z11::RenderBlock> *render_block_list, const float *view_projection)
{
  z11::List<z11::RenderBlock> *block = render_block_list->next();

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  glEnable(GL_MULTISAMPLE);
  glBindFramebuffer(GL_FRAMEBUFFER, eye->framebuffer_id());
  glViewport(0, 0, eye->width(), eye->height());
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (render_block_list->Empty()) goto clean;

  glEnable(GL_DEPTH_TEST);
  glUseProgram(default_shader_.id());
  glUniformMatrix4fv(default_shader_matrix_location_, 1, GL_FALSE, view_projection);
  do {
    glBindVertexArray(block->data()->vertex_array_object());
    // fprintf(stdout, "DrawArrays\n");
    glDrawArrays(GL_LINES, 0, block->data()->GetDataSize() / (sizeof(float) * 3));
    glBindVertexArray(0);
    block = block->next();
  } while (!block->Head());
  glUseProgram(0);

clean:
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDisable(GL_MULTISAMPLE);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, eye->framebuffer_id());
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, eye->copy_framebuffer_id());
  glBlitFramebuffer(0, 0, eye->width(), eye->height(), 0, 0, eye->width(), eye->height(), GL_COLOR_BUFFER_BIT,
                    GL_LINEAR);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
