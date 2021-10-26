#include "simple_box.h"

#include <cglm/cglm.h>
#include <errno.h>
#include <float.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#include "z_window.h"

const char *vertex_shader =
    "#version 410\n"
    "uniform mat4 mvp;\n"
    "layout(location = 0) in vec4 position;\n"
    "layout(location = 1) in vec2 v2UVcoordsIn;\n"
    "layout(location = 2) in vec3 v3NormalIn;\n"
    "out vec2 v2UVcoords;\n"
    "void main()\n"
    "{\n"
    "  v2UVcoords = v2UVcoordsIn;\n"
    "  gl_Position = mvp * position;\n"
    "}\n";

const char *fragment_shader =
    "#version 410 core\n"
    "in vec2 v2UVcoords;\n"
    "out vec4 outputColor;\n"
    "void main()\n"
    "{\n"
    "  outputColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "}\n";

const char *green_fragment_shader =
    "#version 410 core\n"
    "in vec2 v2UVcoords;\n"
    "out vec4 outputColor;\n"
    "void main()\n"
    "{\n"
    "  outputColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
    "}\n";

static const struct z11_cuboid_window_listener cuboid_window_listener = {
    [](void *data, struct z11_cuboid_window *z11_cuboid_window,
       wl_fixed_t width, wl_fixed_t height, wl_fixed_t depth) -> void {
      (void)z11_cuboid_window;
      SimpleBox *app = (SimpleBox *)data;
      app->ResizeEvent(width, height, depth);
    },
};

static void callback(void *data, struct wl_callback *callback,
                     uint32_t callback_data);

static const struct wl_callback_listener callback_listener = {callback};

static void callback(void *data, struct wl_callback *callback,
                     uint32_t callback_data)
{
  (void)callback_data;
  SimpleBox *app = (SimpleBox *)data;
  wl_callback_destroy(callback);
  struct wl_callback *next_cb = app->MainLoop();
  wl_callback_add_listener(next_cb, &callback_listener, app);
}

bool SimpleBox::Init()
{
  int fd;
  socket_ = "z11-0";
  width_ = 10;
  height_ = 10;
  depth_ = 10;
  zwindow_ = new ZWindow<SimpleBox>(this);

  if (zwindow_->Connect(socket_) == false) return false;

  fd = zwindow_->CreateSharedFD(sizeof(Box) + sizeof(Plain));
  if (fd < 0) return false;

  {
    void *data = mmap(NULL, sizeof(Box) + sizeof(Plain), PROT_READ | PROT_WRITE,
                      MAP_SHARED, fd, 0);

    if (data == MAP_FAILED) {
      close(fd);
      return false;
    }

    box_data_ = (Box *)data;
    plain_data_ = (Plain *)((uint8_t *)data + sizeof(Box));
  }

  {
    struct wl_shm_pool *pool;
    pool = wl_shm_create_pool(zwindow_->shm(), fd, sizeof(Box) + sizeof(Plain));
    box_raw_buffer_ = wl_zext_shm_pool_create_raw_buffer(pool, 0, sizeof(Box));
    plain_raw_buffer_ =
        wl_zext_shm_pool_create_raw_buffer(pool, sizeof(Box), sizeof(Plain));
    wl_shm_pool_destroy(pool);
  }

  virtual_object_ =
      z11_compositor_create_virtual_object(zwindow_->compositor());

  box_vertex_buffer_ = z11_opengl_create_vertex_buffer(zwindow_->gl());
  z11_opengl_vertex_buffer_attach(box_vertex_buffer_, box_raw_buffer_,
                                  sizeof(vec3));
  plain_vertex_buffer_ = z11_opengl_create_vertex_buffer(zwindow_->gl());
  z11_opengl_vertex_buffer_attach(plain_vertex_buffer_, plain_raw_buffer_,
                                  sizeof(vec3));

  box_shader_program_ = z11_opengl_create_shader_program(
      zwindow_->gl(), vertex_shader, fragment_shader);
  plain_shader_program_ = z11_opengl_create_shader_program(
      zwindow_->gl(), vertex_shader, green_fragment_shader);

  box_render_component_ =
      z11_opengl_render_component_manager_create_opengl_render_component(
          zwindow_->render_component_manager(), virtual_object_);

  z11_opengl_render_component_attach_vertex_buffer(box_render_component_,
                                                   box_vertex_buffer_);

  z11_opengl_render_component_attach_shader_program(box_render_component_,
                                                    box_shader_program_);

  z11_opengl_render_component_append_vertex_input_attribute(
      box_render_component_, 0,
      Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR3, 0);

  z11_opengl_render_component_set_topology(box_render_component_,
                                           Z11_OPENGL_TOPOLOGY_LINES);

  cuboid_window_ =
      z11_shell_get_cuboid_window(zwindow_->shell(), virtual_object_);
  z11_cuboid_window_add_listener(cuboid_window_, &cuboid_window_listener, this);
  z11_cuboid_window_request_window_size(cuboid_window_, wl_fixed_from_int(30),
                                        wl_fixed_from_int(30),
                                        wl_fixed_from_int(30));

  return true;
}

void SimpleBox::Run()
{
  struct wl_callback *callback = this->MainLoop();
  wl_callback_add_listener(callback, &callback_listener, this);

  int ret;
  while (1) {
    while (wl_display_prepare_read(zwindow_->display()) != 0) {
      if (errno != EAGAIN) break;
      wl_display_dispatch_pending(zwindow_->display());
    }
    ret = wl_display_flush(zwindow_->display());
    if (ret == -1) break;
    wl_display_read_events(zwindow_->display());
    wl_display_dispatch_pending(zwindow_->display());
  }
}

struct wl_callback *SimpleBox::MainLoop()
{
  struct wl_callback *cb;
  float root8 = sqrt(8);
  theta_ += 0.01;

  // draw box

  size_ = (width_ > depth_ ? depth_ : width_) / root8;
  size_ = size_ > (height_ / 2) ? height_ / 2 : size_;

  vec3 vertices[2][4] =  //
      {{

           {-size_, -size_, -size_},  // A
           {+size_, -size_, -size_},  // B
           {+size_, +size_, -size_},  // C
           {-size_, +size_, -size_},  // D
       },
       {

           {-size_, -size_, +size_},  // E
           {+size_, -size_, +size_},  // F
           {+size_, +size_, +size_},  // G
           {-size_, +size_, +size_},  // H
       }};

  vec3 y_axis = {0, 1.0f, 0};
  for (int i = 0; i < 4; i++) {
    glm_vec3_rotate(vertices[0][i], theta_, y_axis);
    glm_vec3_rotate(vertices[1][i], theta_, y_axis);
  }

  for (int i = 0; i < 4; i++) {
    // front (A-B, B-C, B-C, C-A)
    glm_vec3_copy(vertices[0][i], box_data_->edges[i].start);
    glm_vec3_copy(vertices[0][(i + 1) % 4], box_data_->edges[i].end);
    // back (E-F, F-G, G-H, H-E)
    glm_vec3_copy(vertices[1][i], box_data_->edges[i + 4].start);
    glm_vec3_copy(vertices[1][(i + 1) % 4], box_data_->edges[i + 4].end);
    // others (A-E, B-F, C-G, D-H)
    glm_vec3_copy(vertices[0][i], box_data_->edges[i + 8].start);
    glm_vec3_copy(vertices[1][i], box_data_->edges[i + 8].end);
  }

  z11_opengl_vertex_buffer_attach(box_vertex_buffer_, box_raw_buffer_,
                                  sizeof(vec3));

  // draw highlight plain

  float min_distance = FLT_MAX;
  float distance = 0;
  const int face_set_count = 6;
  Face face_set[face_set_count] = {front, back, right, left, top, bottom};
  Face intersection_face = none;

  for (int i = 0; i < face_set_count; i++) {
    distance = this->RayIntersection(face_set[i]);
    if (distance > 0 && distance < min_distance) {
      min_distance = distance;
      intersection_face = face_set[i];
    }
  }

  bool render_plain = true;

  if (ray_focus_ == false) render_plain = false;

  switch (intersection_face) {
    case front:  // ABCD
      glm_vec3_copy(vertices[0][0], plain_data_->triangles[0].points[0]);  // A
      glm_vec3_copy(vertices[0][1], plain_data_->triangles[0].points[1]);  // B
      glm_vec3_copy(vertices[0][2], plain_data_->triangles[0].points[2]);  // C
      glm_vec3_copy(vertices[0][0], plain_data_->triangles[1].points[0]);  // A
      glm_vec3_copy(vertices[0][3], plain_data_->triangles[1].points[1]);  // D
      glm_vec3_copy(vertices[0][2], plain_data_->triangles[1].points[2]);  // C
      break;
    case back:  // EFGH
      glm_vec3_copy(vertices[1][0], plain_data_->triangles[0].points[0]);  // E
      glm_vec3_copy(vertices[1][1], plain_data_->triangles[0].points[1]);  // F
      glm_vec3_copy(vertices[1][2], plain_data_->triangles[0].points[2]);  // G
      glm_vec3_copy(vertices[1][0], plain_data_->triangles[1].points[0]);  // E
      glm_vec3_copy(vertices[1][3], plain_data_->triangles[1].points[1]);  // H
      glm_vec3_copy(vertices[1][2], plain_data_->triangles[1].points[2]);  // G
      break;
    case right:  // BFGC
      glm_vec3_copy(vertices[0][1], plain_data_->triangles[0].points[0]);  // B
      glm_vec3_copy(vertices[1][1], plain_data_->triangles[0].points[1]);  // F
      glm_vec3_copy(vertices[1][2], plain_data_->triangles[0].points[2]);  // G
      glm_vec3_copy(vertices[0][1], plain_data_->triangles[1].points[0]);  // B
      glm_vec3_copy(vertices[0][2], plain_data_->triangles[1].points[1]);  // C
      glm_vec3_copy(vertices[1][2], plain_data_->triangles[1].points[2]);  // G
      break;
    case left:  // AEHD
      glm_vec3_copy(vertices[0][0], plain_data_->triangles[0].points[0]);  // A
      glm_vec3_copy(vertices[1][0], plain_data_->triangles[0].points[1]);  // E
      glm_vec3_copy(vertices[1][3], plain_data_->triangles[0].points[2]);  // H
      glm_vec3_copy(vertices[0][0], plain_data_->triangles[1].points[0]);  // A
      glm_vec3_copy(vertices[0][3], plain_data_->triangles[1].points[1]);  // D
      glm_vec3_copy(vertices[1][3], plain_data_->triangles[1].points[2]);  // H
      break;
    case top:  // CGHD
      glm_vec3_copy(vertices[0][2], plain_data_->triangles[0].points[0]);  // C
      glm_vec3_copy(vertices[1][2], plain_data_->triangles[0].points[1]);  // G
      glm_vec3_copy(vertices[1][3], plain_data_->triangles[0].points[2]);  // H
      glm_vec3_copy(vertices[0][2], plain_data_->triangles[1].points[0]);  // C
      glm_vec3_copy(vertices[0][3], plain_data_->triangles[1].points[1]);  // D
      glm_vec3_copy(vertices[1][3], plain_data_->triangles[1].points[2]);  // H
      break;
    case bottom:  // BFEA
      glm_vec3_copy(vertices[0][1], plain_data_->triangles[0].points[0]);  // B
      glm_vec3_copy(vertices[1][1], plain_data_->triangles[0].points[1]);  // F
      glm_vec3_copy(vertices[1][0], plain_data_->triangles[0].points[2]);  // E
      glm_vec3_copy(vertices[0][1], plain_data_->triangles[1].points[0]);  // B
      glm_vec3_copy(vertices[0][0], plain_data_->triangles[1].points[1]);  // A
      glm_vec3_copy(vertices[1][0], plain_data_->triangles[1].points[2]);  // E
      break;
    case none:
      render_plain = false;
      break;
  }

  if (plain_render_component_)
    z11_opengl_render_component_destroy(plain_render_component_);

  if (render_plain) {
    plain_render_component_ =
        z11_opengl_render_component_manager_create_opengl_render_component(
            zwindow_->render_component_manager(), virtual_object_);
    z11_opengl_render_component_attach_vertex_buffer(plain_render_component_,
                                                     plain_vertex_buffer_);
    z11_opengl_render_component_attach_shader_program(plain_render_component_,
                                                      plain_shader_program_);
    z11_opengl_render_component_append_vertex_input_attribute(
        plain_render_component_, 0,
        Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR3, 0);
    z11_opengl_render_component_set_topology(plain_render_component_,
                                             Z11_OPENGL_TOPOLOGY_TRIANGLES);
  } else {
    plain_render_component_ = nullptr;
  }

  cb = z11_virtual_object_frame(virtual_object_);
  z11_virtual_object_commit(virtual_object_);
  return cb;
}

void SimpleBox::ResizeEvent(wl_fixed_t width, wl_fixed_t height,
                            wl_fixed_t depth)
{
  width_ = wl_fixed_to_double(width);
  height_ = wl_fixed_to_double(height);
  depth_ = wl_fixed_to_double(depth);
}

void SimpleBox::HandleRayEnter(struct z11_ray *ray, uint32_t serial,
                               struct z11_cuboid_window *cuboid_window,
                               float ray_origin_x, float ray_origin_y,
                               float ray_origin_z, float ray_direction_x,
                               float ray_direction_y, float ray_direction_z)
{
  (void)ray;
  (void)serial;
  (void)cuboid_window;
  ray_.origin[0] = ray_origin_x;
  ray_.origin[1] = ray_origin_y;
  ray_.origin[2] = ray_origin_z;
  ray_.direction[0] = ray_direction_x;
  ray_.direction[1] = ray_direction_y;
  ray_.direction[2] = ray_direction_z;
  ray_focus_ = true;
}

void SimpleBox::HandleRayMotion(struct z11_ray *ray, uint32_t time,
                                float ray_origin_x, float ray_origin_y,
                                float ray_origin_z, float ray_direction_x,
                                float ray_direction_y, float ray_direction_z)
{
  (void)ray;
  (void)time;
  ray_.origin[0] = ray_origin_x;
  ray_.origin[1] = ray_origin_y;
  ray_.origin[2] = ray_origin_z;
  ray_.direction[0] = ray_direction_x;
  ray_.direction[1] = ray_direction_y;
  ray_.direction[2] = ray_direction_z;
  ray_focus_ = true;
}

void SimpleBox::HandleRayLeave(struct z11_ray *ray, uint32_t serial,
                               struct z11_cuboid_window *cuboid_window)
{
  (void)ray;
  (void)serial;
  (void)cuboid_window;
  ray_focus_ = false;
}

void SimpleBox::HandleRayButton(struct z11_ray *ray, uint32_t serial,
                                uint32_t time, uint32_t button, uint32_t state)
{
  (void)ray;
  (void)time;
  (void)button;
  if (state == 1)
    z11_cuboid_window_move(cuboid_window_, zwindow_->seat(), serial);
}

float SimpleBox::RayIntersection(Face face)
{
  if (ray_focus_ == false) return -1;
  Ray ray;
  vec3 y_axis = {0, 1, 0};
  vec3 x_axis = {1, 0, 0};
  glm_vec3_copy(ray_.origin, ray.origin);
  glm_vec3_copy(ray_.direction, ray.direction);

  glm_vec3_rotate(ray.origin, -theta_, y_axis);
  glm_vec3_rotate(ray.direction, -theta_, y_axis);

  switch (face) {
    case front:
      break;
    case back:
      glm_vec3_rotate(ray.origin, M_PI, y_axis);
      glm_vec3_rotate(ray.direction, M_PI, y_axis);
      break;
    case right:
      glm_vec3_rotate(ray.origin, M_PI_2, y_axis);
      glm_vec3_rotate(ray.direction, M_PI_2, y_axis);
      break;
    case left:
      glm_vec3_rotate(ray.origin, -M_PI_2, y_axis);
      glm_vec3_rotate(ray.direction, -M_PI_2, y_axis);
      break;
    case top:
      glm_vec3_rotate(ray.origin, -M_PI_2, x_axis);
      glm_vec3_rotate(ray.direction, -M_PI_2, x_axis);
      break;
    case bottom:
      glm_vec3_rotate(ray.origin, M_PI_2, x_axis);
      glm_vec3_rotate(ray.direction, M_PI_2, x_axis);
      break;
    case none:
      return -1;
  }

  if (ray.origin[2] >= -size_) return -1;

  if (ray.direction[2] == 0) return -1;
  float len = (-size_ - ray.origin[2]) / ray.direction[2];

  if (len <= 0) return -1;

  float x = ray.origin[0] + ray.direction[0] * len;
  float y = ray.origin[1] + ray.direction[1] * len;

  if (-size_ < x && x < size_ && -size_ < y && y < size_) return len;

  return -1;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void SimpleBox::HandleKeyboardKeymap(struct z11_keyboard *keyboard,
                                     uint32_t format, int fd, uint32_t size)
{}

void SimpleBox::HandleKeyboardEnter(struct z11_keyboard *keyboard,
                                    uint32_t serial,
                                    struct z11_cuboid_window *cuboid_window,
                                    struct wl_array *keys)
{}

void SimpleBox::HandleKeyboardLeave(struct z11_keyboard *keyboard,
                                    uint32_t serial,
                                    struct z11_cuboid_window *cuboid_window)
{}

void SimpleBox::HandleKeyboardKey(struct z11_keyboard *keyboard,
                                  uint32_t serial, uint32_t time, uint32_t key,
                                  uint32_t state)
{}

void SimpleBox::HandleKeyboardModifiers(struct z11_keyboard *keyboard,
                                        uint32_t serial,
                                        uint32_t mods_depressed,
                                        uint32_t mods_latached,
                                        uint32_t mods_locked, uint32_t group)
{}
#pragma GCC diagnostic pop

int main()
{
  SimpleBox *app = new SimpleBox();
  if (app->Init() == false) return 1;
  app->Run();
  return 0;
}
