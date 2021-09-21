#include "renderer.h"

#include <libzazen.h>

void Renderer::Render(Eye *eye,
                      ZServer::RenderStateIterator *render_state_iterator)
{
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  glEnable(GL_MULTISAMPLE);
  glBindFramebuffer(GL_FRAMEBUFFER, eye->framebuffer_id());
  glViewport(0, 0, eye->width(), eye->height());
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  struct zazen_opengl_render_component_back_state *render_state =
      render_state_iterator->Next();
  if (render_state == nullptr) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_MULTISAMPLE);
    return;
  }

  glEnable(GL_DEPTH_TEST);
  do {
    glUseProgram(render_state->shader_program_id);
    Matrix4 model_matrix = Matrix4(render_state->model_matrix);
    Matrix4 view_projection_matrix = eye->view_projection().get();
    GLint model_view_projection_matrix_location =
        glGetUniformLocation(render_state->shader_program_id, "mvp");
    glUniformMatrix4fv(model_view_projection_matrix_location, 1, GL_FALSE,
                       (view_projection_matrix * model_matrix).get());
    glBindVertexArray(render_state->vertex_array_id);
    glBindTexture(GL_TEXTURE_2D, render_state->texture_2d_id);
    glDrawArrays(
        render_state->topology_mode, 0,
        render_state->vertex_buffer_size / render_state->vertex_stride);
    render_state = render_state_iterator->Next();
  } while (render_state != nullptr);
  glUseProgram(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindVertexArray(0);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDisable(GL_MULTISAMPLE);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, eye->framebuffer_id());
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, eye->resolve_framebuffer_id());
  glBlitFramebuffer(0, 0, eye->width(), eye->height(), 0, 0, eye->width(),
                    eye->height(), GL_COLOR_BUFFER_BIT, GL_LINEAR);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
