#include "png_viewer.h"

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
    "uniform sampler2D myTexture;\n"
    "in vec2 v2UVcoords;\n"
    "out vec4 outputColor;\n"
    "void main()\n"
    "{\n"
    "  outputColor = texture(myTexture, v2UVcoords);\n"
    "}\n";

static const struct z11_cuboid_window_listener cuboid_window_listener = {
    [](void *data, struct z11_cuboid_window *z11_cuboid_window,
       wl_fixed_t width, wl_fixed_t height, wl_fixed_t depth) -> void {
      (void)z11_cuboid_window;
      PngViewer *app = (PngViewer *)data;
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
  PngViewer *app = (PngViewer *)data;
  wl_callback_destroy(callback);
  struct wl_callback *next_cb = app->MainLoop();
  wl_callback_add_listener(next_cb, &callback_listener, app);
}

static bool paint_texture(ColorBGRA *texture, uint8_t *png_data, uint32_t width,
                          uint32_t height, uint32_t ch)
{
  if (ch == 4) {
    ColorRGBA *data = (ColorRGBA *)png_data;
    for (uint32_t y = 0; y < height; y++) {
      for (uint32_t x = 0; x < width; x++) {
        texture->b = data->b;
        texture->g = data->g;
        texture->r = data->r;
        texture->a = data->a;
        data++;
        texture++;
      }
    }
  } else if (ch == 3) {
    ColorRGB *data = (ColorRGB *)png_data;
    for (uint32_t y = 0; y < height; y++) {
      for (uint32_t x = 0; x < width; x++) {
        texture->b = data->b;
        texture->g = data->g;
        texture->r = data->r;
        texture->a = UINT8_MAX;
        data++;
        texture++;
      }
    }
  } else {
    return false;
  }
  return true;
}

PngViewer::PngViewer(const char *filename) { this->filename_ = filename; }

bool PngViewer::Init()
{
  int fd;
  int texture_size;
  void *shm_data;
  socket_ = "z11-0";
  width_ = 10;
  height_ = 10;
  depth_ = 10;
  png_loader_ = new PngLoader(filename_);
  if (png_loader_->Load() == false) return false;
  zwindow_ = new ZWindow<PngViewer>(this);

  if (zwindow_->Connect(socket_) == false) return false;

  texture_size =
      sizeof(ColorBGRA) * png_loader_->width() * png_loader_->height();

  fd = zwindow_->CreateSharedFD(sizeof(Panel) + texture_size);

  shm_data = mmap(NULL, sizeof(Panel) + texture_size, PROT_READ | PROT_WRITE,
                  MAP_SHARED, fd, 0);
  if (shm_data == MAP_FAILED) {
    close(fd);
    return false;
  }

  panel_data_ = (Panel *)shm_data;
  texture_data_ = (ColorBGRA *)((char *)shm_data + sizeof(Panel));

  {
    struct wl_shm_pool *pool =
        wl_shm_create_pool(zwindow_->shm(), fd, sizeof(Panel) + texture_size);
    panel_raw_buffer_ =
        wl_zext_shm_pool_create_raw_buffer(pool, 0, sizeof(Panel));
    texture_raw_buffer_ =
        wl_zext_shm_pool_create_raw_buffer(pool, sizeof(Panel), texture_size);
    wl_shm_pool_destroy(pool);
  }

  virtual_object_ =
      z11_compositor_create_virtual_object(zwindow_->compositor());

  panel_vertex_buffer_ = z11_opengl_create_vertex_buffer(zwindow_->gl());

  texture_ = z11_opengl_create_texture_2d(zwindow_->gl());

  shader_program_ = z11_opengl_create_shader_program(
      zwindow_->gl(), vertex_shader, fragment_shader);

  render_component_ =
      z11_opengl_render_component_manager_create_opengl_render_component(
          zwindow_->render_component_manager(), virtual_object_);

  z11_opengl_render_component_attach_vertex_buffer(render_component_,
                                                   panel_vertex_buffer_);

  z11_opengl_render_component_attach_texture_2d(render_component_, texture_);

  z11_opengl_render_component_attach_shader_program(render_component_,
                                                    shader_program_);

  z11_opengl_render_component_append_vertex_input_attribute(
      render_component_, 0,
      Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR3,
      offsetof(Vertex, p));
  z11_opengl_render_component_append_vertex_input_attribute(
      render_component_, 1,
      Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR2,
      offsetof(Vertex, uv));

  z11_opengl_render_component_set_topology(render_component_,
                                           Z11_OPENGL_TOPOLOGY_TRIANGLES);

  paint_texture(texture_data_, png_loader_->data(), png_loader_->width(),
                png_loader_->height(), png_loader_->channel());

  z11_opengl_texture_2d_set_image(texture_, texture_raw_buffer_,
                                  Z11_OPENGL_TEXTURE_2D_FORMAT_ARGB8888,
                                  png_loader_->width(), png_loader_->height());

  cuboid_window_ =
      z11_shell_get_cuboid_window(zwindow_->shell(), virtual_object_);
  z11_cuboid_window_add_listener(cuboid_window_, &cuboid_window_listener, this);
  z11_cuboid_window_request_window_size(cuboid_window_, wl_fixed_from_int(30),
                                        wl_fixed_from_int(30),
                                        wl_fixed_from_int(30));

  return true;
}

void PngViewer::Run()
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

struct wl_callback *PngViewer::MainLoop()
{
  struct wl_callback *cb;
  float image_raw_width = png_loader_->width();
  float image_raw_height = png_loader_->width();
  float min_w_d = (width_ > depth_ ? depth_ : width_);
  float root3 = sqrt(3);
  float image_width = min_w_d * root3 / 2;
  float image_height = image_width * image_raw_height / image_raw_width;
  if (image_height > height_) {
    image_height = height_;
    image_width = image_height * image_raw_width / image_raw_height;
  }

  theta_ += 0.01;

  float y = image_height / 2;
  float a = (-image_width / 2 * cos(theta_) + depth_ / 2 * sin(theta_));
  float b = (-image_width / 2 * sin(theta_) - depth_ / 2 * cos(theta_));
  float c = (image_width / 2 * cos(theta_) + depth_ / 2 * sin(theta_));
  float d = (image_width / 2 * sin(theta_) - depth_ / 2 * cos(theta_));

  Vertex A = {{a, +y, b}, {0, 0}};
  Vertex B = {{a, -y, b}, {0, 1}};
  Vertex C = {{c, -y, d}, {1, 1}};
  Vertex D = {{c, +y, d}, {1, 0}};

  panel_data_->triangles[0].vertices[0] = A;
  panel_data_->triangles[0].vertices[1] = B;
  panel_data_->triangles[0].vertices[2] = C;
  panel_data_->triangles[1].vertices[0] = C;
  panel_data_->triangles[1].vertices[1] = D;
  panel_data_->triangles[1].vertices[2] = A;

  z11_opengl_vertex_buffer_attach(panel_vertex_buffer_, panel_raw_buffer_,
                                  sizeof(Vertex));
  cb = z11_virtual_object_frame(virtual_object_);
  z11_virtual_object_commit(virtual_object_);
  return cb;
}

void PngViewer::ResizeEvent(wl_fixed_t width, wl_fixed_t height,
                            wl_fixed_t depth)
{
  width_ = wl_fixed_to_double(width);
  height_ = wl_fixed_to_double(height);
  depth_ = wl_fixed_to_double(depth);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void PngViewer::HandleRayEnter(struct z11_ray *ray, uint32_t serial,
                               struct z11_cuboid_window *cuboid_window,
                               float ray_origin_x, float ray_origin_y,
                               float ray_origin_z, float ray_direction_x,
                               float ray_direction_y, float ray_direction_z)
{}

void PngViewer::HandleRayMotion(struct z11_ray *ray, uint32_t time,
                                float ray_origin_x, float ray_origin_y,
                                float ray_origin_z, float ray_direction_x,
                                float ray_direction_y, float ray_direction_z)
{}

void PngViewer::HandleRayLeave(struct z11_ray *ray, uint32_t serial,
                               struct z11_cuboid_window *cuboid_window)
{}

void PngViewer::HandleRayButton(struct z11_ray *ray, uint32_t serial,
                                uint32_t time, uint32_t button, uint32_t state)
{}

void PngViewer::HandleKeyboardEnter(struct z11_keyboard *keyboard,
                                    uint32_t serial,
                                    struct z11_cuboid_window *cuboid_window,
                                    struct wl_array *keys)
{}

void PngViewer::HandleKeyboardLeave(struct z11_keyboard *keyboard,
                                    uint32_t serial,
                                    struct z11_cuboid_window *cuboid_window)
{}

void PngViewer::HandleKeyboardKey(struct z11_keyboard *keyboard,
                                  uint32_t serial, uint32_t time, uint32_t key,
                                  uint32_t state)
{}

void PngViewer::HandleKeyboardModifiers(struct z11_keyboard *keyboard,
                                        uint32_t serial,
                                        uint32_t mods_depressed,
                                        uint32_t mods_latached,
                                        uint32_t mods_locked, uint32_t group)
{}
#pragma GCC diagnostic pop

int main(int argc, char const *argv[])
{
  if (argc <= 1) {
    fprintf(stderr, "help:\n\t%s <filename>\n", argv[0]);
    exit(1);
  }
  const char *filename = argv[1];
  PngViewer *app = new PngViewer(filename);
  if (app->Init() == false) return 1;
  app->Run();
  return 0;
}
