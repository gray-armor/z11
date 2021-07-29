#include "eye.h"

#include <GL/glew.h>

bool Eye::Init(uint32_t renderWidth, uint32_t renderHeight)
{
  width_ = renderWidth;
  height_ = renderHeight;

  if (CreateFramebuffer() == false) return false;
  return true;
}

bool Eye::CreateFramebuffer()
{
  glGenFramebuffers(1, &framebuffer_id_);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id_);

  glGenRenderbuffers(1, &depthbuffer_id_);
  glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer_id_);
  glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, width_, height_);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthbuffer_id_);

  glGenTextures(1, &texture_id_);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture_id_);
  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, width_, height_, true);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture_id_, 0);

  glGenFramebuffers(1, &resolve_framebuffer_id_);
  glBindFramebuffer(GL_FRAMEBUFFER, resolve_framebuffer_id_);

  glGenTextures(1, &resolve_texture_id_);
  glBindTexture(GL_TEXTURE_2D, resolve_texture_id_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolve_texture_id_, 0);

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) return false;

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return true;
}
