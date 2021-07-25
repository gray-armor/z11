#include <SDL2/SDL.h>
#include <libz11.h>

#include "eye.h"
#include "renderer.h"
#include "sdl.h"

class Main
{
 public:
  Main();
  bool Init();
  void RunMainLoop();
  void Shutdown();

 private:
  z11::Compositor *compositor_;
  Eye *left_eye_;
  Eye *right_eye_;
  Renderer *renderer_;

  SDLHead *head_;
  bool run_;

 private:
  void SetEyeProjection();
};

Main::Main() : run_(false)
{
  // TODO: handle args and envs
}

bool Main::Init()
{
  compositor_ = z11::Compositor::Create();
  if (compositor_ == nullptr) return false;

  head_ = new SDLHead();
  if (head_->Init() == false) return false;

  GLenum glewError = glewInit();
  if (glewError != GLEW_OK) {
    fprintf(stdout, "%s - Error initializing GLEW! %s\n", __FUNCTION__, glewGetErrorString(glewError));
    return false;
  }
  glGetError();

  renderer_ = new Renderer();
  if (renderer_->Init() == false) return false;

  left_eye_ = new Eye();
  right_eye_ = new Eye();
  if (left_eye_->Init(320, 320) == false) return false;
  if (right_eye_->Init(320, 320) == false) return false;
  SetEyeProjection();

  if (head_->InitGL(left_eye_, right_eye_) == false) return false;

  return true;
}

void Main::RunMainLoop()
{
  run_ = true;
  while (run_) {
    compositor_->ProcessEvents();

    run_ = head_->ProcessEvents();

    renderer_->Render(left_eye_, compositor_->render_block_list());
    renderer_->Render(right_eye_, compositor_->render_block_list());

    head_->Draw(left_eye_, right_eye_);
    head_->Swap();
  }
}

void Main::Shutdown() { head_->Shutdown(); }

int main()
{
  Main *main = new Main();
  if (!main->Init()) {
    main->Shutdown();
  }

  main->RunMainLoop();

  main->Shutdown();
  return 0;
}

void Main::SetEyeProjection()
{
  float far = 1000;
  float near = 0.001;
  float e = 2 * (far * near) / (near - far);
  float f = (far + near) / (far - near);

  Matrix4 projection_left = Matrix4(  //
      1, 0, 0, 0,                     //
      0, 1, 0, 0,                     //
      0, 0, f, 1,                     //
      0, 0, e, 0                      //
  );

  Matrix4 projection_right = Matrix4(  //
      1, 0, 0, 0,                      //
      0, 1, 0, 0,                      //
      0, 0, f, 1,                      //
      0, 0, e, 0                       //
  );

  Matrix4 eye_pos_left = Matrix4(  //
      1, 0, 0, 0,                  //
      0, 1, 0, 0,                  //
      0, 0, 1, 0,                  //
      1, 0, 0, 1                   //
  );

  Matrix4 eye_pos_right = Matrix4(  //
      1, 0, 0, 0,                   //
      0, 1, 0, 0,                   //
      0, 0, 1, 0,                   //
      -1, 0, 0, 1                   //
  );

  left_eye_->set_projection(projection_left * eye_pos_left);
  right_eye_->set_projection(projection_right * eye_pos_right);
}
