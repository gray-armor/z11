#ifndef Z11_CLIENT_SIMPLE_BOX_H
#define Z11_CLIENT_SIMPLE_BOX_H

#include <cglm/cglm.h>
#include <wl_zext_client.h>
#include <z11-client-protocol.h>

#include "z_window.h"

typedef struct {
  vec3 start, end;
} Line;

typedef struct {
  Line edges[12];
} Box;

typedef struct {
  vec3 points[3];
} Triangle;

typedef struct {
  Triangle triangles[2];
} Plain;

typedef struct {
  vec3 origin, direction;
} Ray;

enum Face {
  front,
  back,
  right,
  left,
  top,
  bottom,
  none,
};

class SimpleBox
{
 public:
  bool Init();
  void Run();
  struct wl_callback *MainLoop();
  void ResizeEvent(wl_fixed_t width, wl_fixed_t height, wl_fixed_t depth);
  void HandleRayEnter(struct z11_ray *ray, uint32_t serial,
                      struct z11_cuboid_window *cuboid_window,
                      float ray_origin_x, float ray_origin_y,
                      float ray_origin_z, float ray_direction_x,
                      float ray_direction_y, float ray_direction_z);
  void HandleRayMotion(struct z11_ray *ray, uint32_t time, float ray_origin_x,
                       float ray_origin_y, float ray_origin_z,
                       float ray_direction_x, float ray_direction_y,
                       float ray_direction_z);
  void HandleRayLeave(struct z11_ray *ray, uint32_t serial,
                      struct z11_cuboid_window *cuboid_window);
  void HandleRayButton(struct z11_ray *ray, uint32_t serial, uint32_t time,
                       uint32_t button, uint32_t state);
  float RayIntersection(Face face);
  void HandleKeyboardKeymap(struct z11_keyboard *keyboard, uint32_t format,
                            int fd, uint32_t size);
  void HandleKeyboardEnter(struct z11_keyboard *keyboard, uint32_t serial,
                           struct z11_cuboid_window *cuboid_window,
                           struct wl_array *keys);
  void HandleKeyboardLeave(struct z11_keyboard *keyboard, uint32_t serial,
                           struct z11_cuboid_window *cuboid_window);
  void HandleKeyboardKey(struct z11_keyboard *keyboard, uint32_t serial,
                         uint32_t time, uint32_t key, uint32_t state);
  void HandleKeyboardModifiers(struct z11_keyboard *keyboard, uint32_t serial,
                               uint32_t mods_depressed, uint32_t mods_latched,
                               uint32_t mods_locked, uint32_t group);

 private:
  float width_;
  float height_;
  float depth_;
  float size_;
  float theta_;
  const char *socket_;
  ZWindow<SimpleBox> *zwindow_;
  struct z11_virtual_object *virtual_object_;
  Box *box_data_;
  Plain *plain_data_;
  struct wl_zext_raw_buffer *box_raw_buffer_;
  struct wl_zext_raw_buffer *plain_raw_buffer_;
  struct z11_opengl_vertex_buffer *box_vertex_buffer_;
  struct z11_opengl_vertex_buffer *plain_vertex_buffer_;
  struct z11_opengl_shader_program *box_shader_program_;
  struct z11_opengl_shader_program *plain_shader_program_;
  struct z11_opengl_render_component *box_render_component_;
  struct z11_opengl_render_component *plain_render_component_;
  struct z11_cuboid_window *cuboid_window_;
  Ray ray_;
  bool ray_focus_;
};

#endif  //  Z11_CLIENT_SIMPLE_BOX_H
