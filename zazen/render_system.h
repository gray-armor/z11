#ifndef Z11_RENDER_SYSTEM_H
#define Z11_RENDER_SYSTEM_H

#include <GL/glew.h>
#include <libzazen.h>

#include "eye.h"
#include "shader.h"
#include "z_server.h"

class RenderSystem
{
 public:
  void Render(Eye *eye, ZServer::RenderStateIterator *render_state_iterator);
};

#endif  // Z11_RENDER_SYSTEM_H
