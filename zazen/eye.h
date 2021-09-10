#ifndef Z11_EYE_H
#define Z11_EYE_H

#include <GL/glew.h>

#include "matrices.h"

class Eye
{
 public:
  bool Init(uint32_t renderWidth, uint32_t renderHeight);
  void set_view_projection(Matrix4 view_projection);
  Matrix4 view_projection();
  GLuint framebuffer_id();
  GLuint resolve_framebuffer_id();
  GLuint resolve_texture_id();
  uint32_t width();
  uint32_t height();

 private:
  uint32_t width_;
  uint32_t height_;
  Matrix4 view_projection_;
  GLuint framebuffer_id_;
  GLuint resolve_framebuffer_id_;
  GLuint texture_id_;
  GLuint resolve_texture_id_;  // TODO: Remove if unnecessary
  GLuint depthbuffer_id_;

 private:
  bool CreateFramebuffer();
};

inline void Eye::set_view_projection(Matrix4 view_projection)
{
  view_projection_ = view_projection;
}

inline Matrix4 Eye::view_projection() { return view_projection_; }

inline GLuint Eye::framebuffer_id() { return framebuffer_id_; }
inline GLuint Eye::resolve_framebuffer_id() { return resolve_framebuffer_id_; }

inline GLuint Eye::resolve_texture_id() { return resolve_texture_id_; }

inline uint32_t Eye::width() { return width_; }
inline uint32_t Eye::height() { return height_; }

#endif  // Z11_EYE_H
