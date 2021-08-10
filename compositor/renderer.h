#ifndef Z11_RENDERER_H
#define Z11_RENDERER_H

#include <GL/glew.h>
#include <libz11.h>

#include "eye.h"
#include "shader.h"
#include "z_server.h"

class Renderer
{
 public:
  void Render(Eye *eye, ZServer::RenderElementIterator *render_element_iterator);
};

#endif  // Z11_RENDERER_H
