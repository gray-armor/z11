#include <SDL2/SDL.h>
#include <libz11.h>
#include <sys/time.h>

#include "eye.h"
#include "hmd.h"
#include "renderer.h"
#include "sdl.h"

void print_fps(int interval_sec);

class Main
{
 public:
  Main(int argc, char const *argv[]);
  bool Init();
  void RunMainLoop();
  void Shutdown();

 private:
  z11::Compositor *compositor_;
  Eye *left_eye_;
  Eye *right_eye_;
  Renderer *renderer_;

  SDLHead *head_;
  HMD *hmd_;
  bool run_;

  // parameters
  bool print_fps_;

 private:
  void PrintUsage(int error_code);
  void SetEyeProjection();
};

Main::Main(int argc, char const *argv[]) : run_(false), print_fps_(false)
{
  for (int i = 1; i < argc; i++) {
    if (strcmp("-fps", argv[i]) == 0) {
      print_fps_ = true;
    } else if (strcmp("-h", argv[i]) == 0) {
      PrintUsage(EXIT_SUCCESS);
    } else {
      PrintUsage(EXIT_FAILURE);
    }
  }
}

bool Main::Init()
{
  compositor_ = z11::Compositor::Create();
  if (compositor_ == nullptr) return false;

  head_ = new SDLHead();
  if (head_->Init() == false) return false;

  hmd_ = new HMD();
  if (hmd_->Init() == false) return false;

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
  if (left_eye_->Init(hmd_->display_width_, hmd_->display_height_) == false) return false;
  if (right_eye_->Init(hmd_->display_width_, hmd_->display_height_) == false) return false;
  // if (left_eye_->Init(320, 320) == false) return false;
  // if (right_eye_->Init(320, 320) == false) return false;
  SetEyeProjection();

  if (head_->InitGL(left_eye_, right_eye_) == false) return false;

  if (hmd_->InitGL(left_eye_, right_eye_) == false) return false;

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

    hmd_->Draw(left_eye_, right_eye_);
    hmd_->Submit();
    hmd_->UpdateHeadPose();

    head_->Draw(left_eye_, right_eye_);
    head_->Swap();
    if (print_fps_) print_fps(4);
  }
}

void Main::Shutdown() { head_->Shutdown(); }

void Main::PrintUsage(int error_code)
{
  fprintf(stderr,
          "Usage: z11 [OPTIONS]\n"
          "\n"
          " -fps \tPrint fps\n");
  exit(error_code);
}

int main(int argc, char const *argv[])
{
  Main *main = new Main(argc, argv);
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

void print_fps(int interval_sec)
{
  static struct timeval base = {0, 0};
  static int count = 0;
  if (base.tv_sec == 0 && base.tv_usec == 0) {
    gettimeofday(&base, NULL);
  }
  count++;

  struct timeval now;
  gettimeofday(&now, NULL);

  if ((now.tv_sec - base.tv_sec) * 1000000 + now.tv_usec - base.tv_usec > 1000000 * interval_sec) {  // 60 hz
    fprintf(stdout, "%d fps\n", count / interval_sec);
    count = 0;
    base = now;
  }
}
