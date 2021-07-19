#include <libz11.h>
#include <stdio.h>

class Main
{
 public:
  Main();
  bool Init();
  void RunMainLoop();
  void Shutdown();
  void ProcessRenderBlock();

 private:
  z11::Compositor *compositor_;
  bool run_;
};

Main::Main() : compositor_(nullptr), run_(false)
{
  // TODO: handle args and envs
}

bool Main::Init()
{
  compositor_ = z11::Compositor::Create();
  if (compositor_ == nullptr) return false;
  return true;
}

void Main::RunMainLoop()
{
  run_ = true;
  while (run_) {
    compositor_->ProcessEvents();
    ProcessRenderBlock();
  }
}

void Main::Shutdown() {}

void Main::ProcessRenderBlock()
{
  fprintf(stdout, "\033[2J\033[0;0H");  // clear console
  fprintf(stdout, "Listing render blocks\n");
  struct wl_list *render_blocks = compositor_->GetRenderBlocks();
  z11::RenderBlock *render_block;
#pragma GCC diagnostic ignored "-Winvalid-offsetof"  // FIXME: use custom list object suitable to c++
  wl_list_for_each(render_block, render_blocks, link_) { fprintf(stdout, "%s\n", render_block->sample_attr); }
  fflush(stdout);
}

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
