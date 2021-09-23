#ifndef Z11_CLIENT_SIMPLE_BOX_H
#define Z11_CLIENT_SIMPLE_BOX_H

#include <wl_zext_client.h>
#include <z11-client-protocol.h>

#include "z_window.h"

typedef struct {
  float x, y, z;
} Point;

typedef struct {
  Point start, end;
} Line;

typedef struct {
  Line edges[12];
} Box;

class SimpleBox
{
 public:
  bool Init();
  void Run();
  struct wl_callback *MainLoop();
  void ResizeEvent(wl_fixed_t width, wl_fixed_t height, wl_fixed_t depth);

 private:
  float width_;
  float height_;
  float depth_;
  float theta_;
  const char *socket_;
  ZWindow *zwindow_;
  struct z11_virtual_object *virtual_object_;
  Box *box_data_;
  struct wl_zext_raw_buffer *box_raw_buffer_;
  struct z11_opengl_vertex_buffer *box_vertex_buffer_;
  struct z11_opengl_shader_program *shader_program_;
  struct z11_opengl_render_component *render_component_;
  struct z11_cuboid_window *cuboid_window_;
};

#endif  //  Z11_CLIENT_SIMPLE_BOX_H
