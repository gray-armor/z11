#ifndef Z11_RENDERER_H
#define Z11_RENDERER_H

#include <GL/glew.h>
#include <libz11.h>

#include "eye.h"
#include "shader.h"

class Renderer
{
 public:
  bool Init();
  void Render(Eye *eye, z11::List<z11::RenderBlock> *render_block_list, const float *projection_matrix);

 private:
  Shader default_shader_;
  GLint default_shader_matrix_location_;
};

#endif  // Z11_RENDERER_H
