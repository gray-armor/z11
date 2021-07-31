#ifndef Z11_SDL_H
#define Z11_SDL_H

#include <SDL2/SDL.h>

#include "eye.h"
#include "shader.h"
#include "vectors.h"

struct VertexDataWindow {
  Vector2 position;
  Vector2 texCoord;
  VertexDataWindow(const Vector2 &pos, const Vector2 tex) : position(pos), texCoord(tex) {}
};

class SDLHead
{
 public:
  bool Init();
  bool InitGL();
  bool ProcessEvents();
  void Draw(Eye *left_eye, Eye *right_eye);
  void Swap();
  void Shutdown();

 private:
  SDL_Window *window_;
  SDL_GLContext gl_context_;
  uint32_t window_width_;
  uint32_t window_height_;
  uint32_t index_size_;
  GLuint vertex_array_object_;
  GLuint vertex_buffer_;
  GLuint index_buffer_;
  Shader default_shader_;
};

#endif  // Z11_SDL_H
