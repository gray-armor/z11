#include "renderer.h"

#include <libz11.h>

void Renderer::Render(Eye *eye, ZServer::RenderBlockIterator *render_block_iterator)
{
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  glEnable(GL_MULTISAMPLE);
  glBindFramebuffer(GL_FRAMEBUFFER, eye->framebuffer_id());
  glViewport(0, 0, eye->width(), eye->height());
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  struct z_render_block *render_block = render_block_iterator->Next();
  if (render_block == nullptr) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_MULTISAMPLE);
    return;
  }

  glEnable(GL_DEPTH_TEST);
  do {
    z_render_block_draw(render_block, eye->view_projection().get());
    render_block = render_block_iterator->Next();
  } while (render_block != nullptr);
  glUseProgram(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindVertexArray(0);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDisable(GL_MULTISAMPLE);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, eye->framebuffer_id());
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, eye->resolve_framebuffer_id());
  glBlitFramebuffer(0, 0, eye->width(), eye->height(), 0, 0, eye->width(), eye->height(), GL_COLOR_BUFFER_BIT,
                    GL_LINEAR);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
