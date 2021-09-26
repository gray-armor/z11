#include "z_server.h"

#include <libzazen.h>
#include <wayland-server.h>

bool ZServer::Init()
{
  const char* socket = "z11-0";
  struct zazen_opengl* gl;

  display_ = wl_display_create();
  loop_ = wl_display_get_event_loop(display_);

  compositor_ = zazen_compositor_create(display_);
  if (compositor_ == NULL) return false;

  gl = zazen_opengl_create(display_);
  if (gl == NULL) return false;

  render_component_manager_ =
      zazen_opengl_render_component_manager_create(display_);
  if (render_component_manager_ == NULL) return false;

  shell_ = zazen_shell_create(display_, render_component_manager_);
  if (shell_ == NULL) return false;

  wl_display_init_shm(display_);

  if (wl_display_add_socket(display_, socket) != 0) return false;

  seat_ = zazen_seat_create(display_, render_component_manager_);
  if (seat_ == NULL) return false;

  return true;
}

void ZServer::Poll()
{
  wl_display_flush_clients(display_);
  wl_event_loop_dispatch(loop_, 0);
}

void ZServer::Frame()
{
  zazen_compositor_emit_frame_signal(compositor_);
  wl_display_flush_clients(display_);
}

bool ZServer::GetRayState(struct zazen_ray_back_state* ray_back_state)
{
  return zazen_seat_get_ray_back_state(seat_, ray_back_state);
}

ZServer::RenderStateIterator* ZServer::NewRenderStateIterator()
{
  struct wl_list* render_component_back_state_list =
      zazen_opengl_render_component_manager_get_render_component_back_state_list(
          render_component_manager_);
  return new RenderStateIterator(render_component_back_state_list);
}

void ZServer::DeleteRenderStateIterator(
    RenderStateIterator* render_component_iterator)
{
  delete render_component_iterator;
}

ZServer::RenderStateIterator::RenderStateIterator(struct wl_list* list)
    : list_(list), pos_(list)
{}

struct zazen_opengl_render_component_back_state*
ZServer::RenderStateIterator::Next()
{
  struct zazen_opengl_render_component_back_state* back_state;
  if (pos_->next == list_) return nullptr;
  pos_ = pos_->next;

  back_state = wl_container_of(pos_, back_state, link);
  return back_state;
}

void ZServer::RenderStateIterator::Rewind() { pos_ = list_; }

ZServer::CuboidWindowIterator* ZServer::NewCuboidWindowIterator()
{
  struct wl_list* cuboid_window_back_state_list =
      zazen_shell_get_cuboid_window_back_state_list(shell_);
  return new CuboidWindowIterator(cuboid_window_back_state_list);
}

void ZServer::DeleteCuboidWindowIterator(
    CuboidWindowIterator* cuboid_window_iterator)
{
  delete cuboid_window_iterator;
}

ZServer::CuboidWindowIterator::CuboidWindowIterator(struct wl_list* list)
    : list_(list), pos_(list)
{}

struct zazen_cuboid_window_back_state* ZServer::CuboidWindowIterator::Next()
{
  struct zazen_cuboid_window_back_state* back_state;
  if (pos_->next == list_) return nullptr;
  pos_ = pos_->next;

  back_state = wl_container_of(pos_, back_state, link);
  return back_state;
}

void ZServer::CuboidWindowIterator::Rewind() { pos_ = list_; }
