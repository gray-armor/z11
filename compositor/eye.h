#ifndef Z11_EYE_H
#define Z11_EYE_H

#include <GL/glew.h>
#include <z11/matrices.h>

class Eye
{
 public:
  bool Init(uint32_t renderWidth, uint32_t renderHeight);
  void set_projection(Matrix4 projection);
  Matrix4 projection();
  GLuint framebuffer_id();
  GLuint copy_framebuffer_id();
  GLuint copy_texture_id();
  uint32_t width();
  uint32_t height();

 private:
  uint32_t width_;
  uint32_t height_;
  Matrix4 projection_;
  GLuint framebuffer_id_;
  GLuint copy_framebuffer_id_;
  GLuint texture_id_;
  GLuint copy_texture_id_;
  GLuint depthbuffer_id_;

 private:
  bool CreateFramebuffer();
};

inline void Eye::set_projection(Matrix4 projection) { projection_ = projection; }

inline Matrix4 Eye::projection() { return projection_; }

inline GLuint Eye::framebuffer_id() { return framebuffer_id_; }
inline GLuint Eye::copy_framebuffer_id() { return copy_framebuffer_id_; }

inline GLuint Eye::copy_texture_id() { return copy_texture_id_; }

inline uint32_t Eye::width() { return width_; }
inline uint32_t Eye::height() { return height_; }

#endif  // Z11_EYE_H
