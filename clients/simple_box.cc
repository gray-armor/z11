#include "simple_box.h"

#include <errno.h>
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
  zwindow_ = new ZWindow();

  if (zwindow_->Connect(socket_) == false) return false;

  fd = zwindow_->CreateSharedFD(sizeof(Box));
  if (fd < 0) return false;

  box_data_ =
      (Box *)mmap(NULL, sizeof(Box), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (box_data_ == MAP_FAILED) {
    close(fd);
    return false;
  }

  {
    struct wl_shm_pool *pool;
    pool = wl_shm_create_pool(zwindow_->shm(), fd, sizeof(Box));
    box_raw_buffer_ = wl_shm_pool_create_raw_buffer(pool, 0, sizeof(Box));
    wl_shm_pool_destroy(pool);
  }

  virtual_object_ =
      z11_compositor_create_virtual_object(zwindow_->compositor());

  box_vertex_buffer_ = z11_opengl_create_vertex_buffer(zwindow_->gl());
  z11_opengl_vertex_buffer_attach(box_vertex_buffer_, box_raw_buffer_,
                                  sizeof(Point));

  shader_program_ = z11_opengl_create_shader_program(
      zwindow_->gl(), vertex_shader, fragment_shader);

  render_component_ =
      z11_opengl_render_component_manager_create_opengl_render_component(
          zwindow_->render_component_manager(), virtual_object_);

  z11_opengl_render_component_attach_vertex_buffer(render_component_,
                                                   box_vertex_buffer_);

  z11_opengl_render_component_attach_shader_program(render_component_,
                                                    shader_program_);

  z11_opengl_render_component_append_vertex_input_attribute(
      render_component_, 0,
      Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR3, 0);

  z11_opengl_render_component_set_topology(render_component_,
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
  theta_ += 0.01;

  float root8 = sqrt(8);
  float size = (width_ > depth_ ? depth_ : width_) / root8;
  size = size > (height_ / 2) ? height_ / 2 : size;

  float y = size;
  float a = (size * cos(theta_) - size * sin(theta_));
  float b = (size * sin(theta_) + size * cos(theta_));
  float c = (size * cos(theta_) + size * sin(theta_));
  float d = (size * sin(theta_) - size * cos(theta_));

  Point A = {-a, +y, -b};
  Point B = {+c, +y, +d};
  Point C = {+a, +y, +b};
  Point D = {-c, +y, -d};
  Point E = {-a, -y, -b};
  Point F = {+c, -y, +d};
  Point G = {+a, -y, +b};
  Point H = {-c, -y, -d};
  box_data_->edges[0].start = A;
  box_data_->edges[0].end = B;
  box_data_->edges[1].start = B;
  box_data_->edges[1].end = C;
  box_data_->edges[2].start = C;
  box_data_->edges[2].end = D;
  box_data_->edges[3].start = D;
  box_data_->edges[3].end = A;
  box_data_->edges[4].start = E;
  box_data_->edges[4].end = F;
  box_data_->edges[5].start = F;
  box_data_->edges[5].end = G;
  box_data_->edges[6].start = G;
  box_data_->edges[6].end = H;
  box_data_->edges[7].start = H;
  box_data_->edges[7].end = E;
  box_data_->edges[8].start = A;
  box_data_->edges[8].end = E;
  box_data_->edges[9].start = B;
  box_data_->edges[9].end = F;
  box_data_->edges[10].start = C;
  box_data_->edges[10].end = G;
  box_data_->edges[11].start = D;
  box_data_->edges[11].end = H;

  z11_opengl_vertex_buffer_attach(box_vertex_buffer_, box_raw_buffer_,
                                  sizeof(Point));
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

int main()
{
  SimpleBox *app = new SimpleBox();
  if (app->Init() == false) return 1;
  app->Run();
  return 0;
}
