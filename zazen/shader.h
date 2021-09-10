#ifndef Z11_SHADER_H
#define Z11_SHADER_H

#include <GL/glew.h>

class Shader
{
 public:
  bool Init(const char *shader_name, const char *vertex_shader,
            const char *fragment_shader);
  GLuint id();

 private:
  GLuint id_;
};

inline GLuint Shader::id() { return id_; }

#endif  // Z11_SHADER_H
