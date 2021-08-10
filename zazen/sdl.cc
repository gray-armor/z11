#include "sdl.h"

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <vector>

#include "eye.h"

bool SdlHead::Init()
{
  if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;

  int windowPosX = 700;
  int windowPosY = 100;
  window_width_ = 640;
  window_height_ = 320;

  Uint32 windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  window_ = SDL_CreateWindow("Zazen SDL Window", windowPosX, windowPosY, window_width_, window_height_,
                             windowFlags);
  if (window_ == nullptr) return false;

  gl_context_ = SDL_GL_CreateContext(window_);
  if (gl_context_ == nullptr) return false;

  return true;
}

bool SdlHead::InitGL()
{
  // prepare shader
  if (default_shader_.Init(  //
          "CompanionWindow",

          // vertex shader
          "#version 410 core\n"
          "layout(location = 0) in vec4 position;\n"
          "layout(location = 1) in vec2 v2UVIn;\n"
          "noperspective out vec2 v2UV;\n"
          "void main()\n"
          "{\n"
          "	v2UV = v2UVIn;\n"
          "	gl_Position = position;\n"
          "}\n",

          // fragment shader
          "#version 410 core\n"
          "uniform sampler2D mytexture;\n"
          "noperspective in vec2 v2UV;\n"
          "out vec4 outputColor;\n"
          "void main()\n"
          "{\n"
          // "  outputColor = vec4(1.0, 0.0, 0.0, 1.0);"
          "  outputColor = texture(mytexture, v2UV);\n"
          "}\n") == false)
    return false;

  // create scene
  std::vector<VertexDataWindow> verts;

  // left eye verts
  verts.push_back(VertexDataWindow(Vector2(-1, -1), Vector2(0, 0)));
  verts.push_back(VertexDataWindow(Vector2(0, -1), Vector2(1, 0)));
  verts.push_back(VertexDataWindow(Vector2(-1, 1), Vector2(0, 1)));
  verts.push_back(VertexDataWindow(Vector2(0, 1), Vector2(1, 1)));

  // right eye verts
  verts.push_back(VertexDataWindow(Vector2(0, -1), Vector2(0, 0)));
  verts.push_back(VertexDataWindow(Vector2(1, -1), Vector2(1, 0)));
  verts.push_back(VertexDataWindow(Vector2(0, 1), Vector2(0, 1)));
  verts.push_back(VertexDataWindow(Vector2(1, 1), Vector2(1, 1)));

  GLushort indices[] = {0, 1, 3, 0, 3, 2, 4, 5, 7, 4, 7, 6};
  index_size_ = sizeof(indices) / sizeof(indices[0]);

  glGenVertexArrays(1, &vertex_array_object_);
  glBindVertexArray(vertex_array_object_);

  glGenBuffers(1, &vertex_buffer_);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(VertexDataWindow), &verts[0], GL_STATIC_DRAW);

  glGenBuffers(1, &index_buffer_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_size_ * sizeof(GLushort), &indices[0], GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow),
                        (void *)offsetof(VertexDataWindow, position));

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow),
                        (void *)offsetof(VertexDataWindow, texCoord));

  glBindVertexArray(0);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  return true;
}

void SdlHead::Draw(Eye *left_eye, Eye *right_eye)
{
  // copy left eye
  glBindFramebuffer(GL_READ_FRAMEBUFFER, left_eye->framebuffer_id());
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, left_eye->resolve_framebuffer_id());

  glBlitFramebuffer(0, 0, left_eye->width(), left_eye->height(), 0, 0, left_eye->width(), left_eye->height(),
                    GL_COLOR_BUFFER_BIT, GL_LINEAR);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  // copy right eye
  glBindFramebuffer(GL_READ_FRAMEBUFFER, right_eye->framebuffer_id());
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, right_eye->resolve_framebuffer_id());

  glBlitFramebuffer(0, 0, right_eye->width(), right_eye->height(), 0, 0, right_eye->width(),
                    right_eye->height(), GL_COLOR_BUFFER_BIT, GL_LINEAR);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  // draw from copied buffer to default frame buffer
  glDisable(GL_DEPTH_TEST);
  glViewport(0, 0, window_width_, window_height_);

  glBindVertexArray(vertex_array_object_);
  glUseProgram(default_shader_.id());

  glBindTexture(GL_TEXTURE_2D, left_eye->resolve_texture_id());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glDrawElements(GL_TRIANGLES, index_size_ / 2, GL_UNSIGNED_SHORT, 0);

  glBindTexture(GL_TEXTURE_2D, right_eye->resolve_texture_id());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glDrawElements(GL_TRIANGLES, index_size_ / 2, GL_UNSIGNED_SHORT, (const void *)(uintptr_t)(index_size_));

  glBindVertexArray(0);
  glUseProgram(0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SdlHead::Swap()
{
  // Swap from default framebuffer
  SDL_GL_SwapWindow(window_);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

bool SdlHead::ProcessEvents()
{
  SDL_Event sdlEvent;
  while (SDL_PollEvent(&sdlEvent) != 0) {
    switch (sdlEvent.type) {
      case SDL_QUIT:
        return false;
        break;

      case SDL_WINDOWEVENT:
        switch (sdlEvent.window.event) {
          case SDL_WINDOWEVENT_CLOSE:
            return false;
            break;

          default:
            break;
        }
        break;

      default:
        break;
    }
  }
  return true;
}

void SdlHead::Shutdown()
{
  if (window_) {
    SDL_DestroyWindow(window_);
    window_ = nullptr;
  }

  SDL_Quit();
}
