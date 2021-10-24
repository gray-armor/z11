#include <SDL2/SDL.h>
#include <sys/time.h>

#include "eye.h"
#include "hmd.h"
#include "render_system.h"
#include "sdl.h"
#include "z_server.h"

void print_fps(int interval_sec);

class Main
{
 public:
  Main(int argc, char const *argv[]);
  bool Init();
  void RunMainLoop();
  void Shutdown();

 private:
  ZServer *z_server_;
  Eye *left_eye_;
  Eye *right_eye_;
  RenderSystem *render_system_;

  SdlHead *head_;
  Hmd *hmd_;
  bool run_;

  // parameters
  bool print_fps_;
  bool with_hmd_;

 private:
  void PrintUsage(int error_code);
  void SetEyeProjection();
};

Main::Main(int argc, char const *argv[])
    : run_(false), print_fps_(false), with_hmd_(true)
{
  for (int i = 1; i < argc; i++) {
    if (strcmp("-fps", argv[i]) == 0) {
      print_fps_ = true;
    } else if (strcmp("-no-hmd", argv[i]) == 0) {
      with_hmd_ = false;
    } else if (strcmp("-h", argv[i]) == 0) {
      PrintUsage(EXIT_SUCCESS);
    } else {
      PrintUsage(EXIT_FAILURE);
    }
  }
}

bool Main::Init()
{
  z_server_ = new ZServer();
  if (z_server_->Init() == false) return false;

  head_ = new SdlHead();
  if (head_->Init() == false) return false;

  if (with_hmd_) {
    hmd_ = new Hmd();
    if (hmd_->Init() == false) return false;
  }

  GLenum glewError = glewInit();
  if (glewError != GLEW_OK) {
    fprintf(stdout, "%s - Error initializing GLEW! %s\n", __FUNCTION__,
            glewGetErrorString(glewError));
    return false;
  }
  glGetError();

  render_system_ = new RenderSystem();

  left_eye_ = new Eye();
  right_eye_ = new Eye();
  uint32_t renderWidth;
  uint32_t renderHeight;
  if (with_hmd_) {
    renderWidth = hmd_->display_width();
    renderHeight = hmd_->display_height();
  } else {
    renderWidth = 1280;
    renderHeight = 1280;
    SetEyeProjection();
  }
  if (left_eye_->Init(renderWidth, renderHeight) == false) return false;
  if (right_eye_->Init(renderWidth, renderHeight) == false) return false;

  if (head_->InitGL() == false) return false;

  return true;
}

void Main::RunMainLoop()
{
  run_ = true;
  while (run_) {
    z_server_->Poll();
    z_server_->Frame();

    run_ = head_->ProcessEvents();

    if (with_hmd_) {
      left_eye_->set_view_projection(hmd_->ViewProjectionMatrix(Hmd::kLeftEye));
      right_eye_->set_view_projection(
          hmd_->ViewProjectionMatrix(Hmd::kRightEye));
    }

    ZServer::RenderStateIterator *render_state_iterator =
        z_server_->NewRenderStateIterator();
    render_system_->Render(left_eye_, render_state_iterator);
    render_state_iterator->Rewind();
    render_system_->Render(right_eye_, render_state_iterator);
    z_server_->DeleteRenderStateIterator(render_state_iterator);

    if (with_hmd_) {
      hmd_->Submit(left_eye_, right_eye_);
      hmd_->UpdateHeadPose();
    }

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
          " -fps    \tPrint fps\n"                   //
          " -no-hmd \tWithout head mount display\n"  //
  );
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

  Matrix4 view_left = Matrix4(  //
      1, 0, 0, 0,               //
      0, 1, 0, 0,               //
      0, 0, 1, 0,               //
      1, 0, 0, 1                //
  );

  Matrix4 view_right = Matrix4(  //
      1, 0, 0, 0,                //
      0, 1, 0, 0,                //
      0, 0, 1, 0,                //
      -1, 0, 0, 1                //
  );

  left_eye_->set_view_projection(projection_left * view_left);
  right_eye_->set_view_projection(projection_right * view_right);
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

  if ((now.tv_sec - base.tv_sec) * 1000000 + now.tv_usec - base.tv_usec >
      1000000 * interval_sec) {  // 60 hz
    fprintf(stdout, "%d fps\n", count / interval_sec);
    count = 0;
    base = now;
  }
}
