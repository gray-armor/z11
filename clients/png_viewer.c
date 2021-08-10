#define _GNU_SOURCE 1
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>
#include <wayland-client.h>

#include "helper.h"
#include "z11-client-protocol.h"

typedef struct {
  Point point;
  UVCoord uv;
} Vertex;

typedef struct {
  Vertex v1, v2, v3;
} Triangle;

const char *vertex_shader;
const char *fragment_shader;
static void paint_vertex(Triangle *triangles, uint32_t width, uint32_t height, int x, int y, int z,
                         float theta);
static void paint_texture(ColorBGRA *texture, uint8_t *png_data, uint32_t width, uint32_t height,
                          uint32_t ch);

int main(int argc, char const *argv[])
{
  uint32_t width, height, ch;
  int x = 0;
  int y = 0;
  int z = 40;
  double del_theta = 0;
  double theta = 0;
  uint8_t *png_data;

  if (argc <= 1) {
    fprintf(stderr, "help:\n\t%s <filename>\n", argv[0]);
    exit(1);
  }
  if (argc > 2) x = atoi(argv[2]);
  if (argc > 3) y = atoi(argv[3]);
  if (argc > 4) z = atoi(argv[4]);
  if (argc > 5) del_theta = atof(argv[5]) / 100;

  png_data = z_helper_png(argv[1], &width, &height, &ch);
  if (png_data == NULL) exit(1);
  fprintf(stderr, "Loaded PNG image (%d x %d)\n", width, height);

  struct z11_global *global = z_helper_global();

  // prepare shared memory
  int size_of_triangles = sizeof(Triangle) * 2;
  int size_of_texture = sizeof(ColorBGRA) * width * height;
  int mem_size = size_of_texture + size_of_triangles;
  int fd = create_shared_fd(mem_size);
  void *shm_data = mmap(NULL, mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (shm_data == MAP_FAILED) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
    fprintf(stderr, "line mmap failed: %m\n");
#pragma GCC diagnostic pop
    close(fd);
    exit(1);
  }
  Triangle *triangle_data = (Triangle *)shm_data;
  ColorBGRA *texture_data = (ColorBGRA *)((char *)shm_data + size_of_triangles);

  // prepare buffer objects
  struct wl_shm_pool *pool = wl_shm_create_pool(global->shm, fd, mem_size);
  struct wl_raw_buffer *triangle_buffer = wl_shm_pool_create_raw_buffer(pool, 0, size_of_triangles);
  struct wl_raw_buffer *texture_buffer =
      wl_shm_pool_create_raw_buffer(pool, size_of_triangles, size_of_texture);
  wl_shm_pool_destroy(pool);

  // prepare vertex buffers
  struct z11_gl_vertex_buffer *vertex_buffer = z11_gl_create_vertex_buffer(global->gl);

  // prepre texture
  struct z11_gl_texture_2d *texture_2d = z11_gl_create_texture_2d(global->gl);

  // prepare shaders
  struct z11_gl_shader_program *shader_program =
      z11_gl_create_shader_program(global->gl, vertex_shader, fragment_shader);

  // prepare render element
  struct z11_render_element *render_element = z11_compositor_create_render_element(global->compositor);
  z11_render_element_attach_vertex_buffer(render_element, vertex_buffer, sizeof(Vertex));
  z11_render_element_attach_shader_program(render_element, shader_program);
  z11_render_element_attach_texture_2d(render_element, texture_2d);
  z11_render_element_append_vertex_input_attribute(
      render_element, 0, Z11_GL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR3, offsetof(Vertex, point));
  z11_render_element_append_vertex_input_attribute(
      render_element, 1, Z11_GL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR2, offsetof(Vertex, uv));
  z11_render_element_set_topology(render_element, Z11_GL_TOPOLOGY_TRIANGLES);

  // render
  paint_vertex(triangle_data, width, height, x, y, z, theta);
  paint_texture(texture_data, png_data, width, height, ch);
  z11_gl_vertex_buffer_allocate(vertex_buffer, size_of_triangles, triangle_buffer);
  z11_gl_texture_2d_set_image(texture_2d, texture_buffer, Z11_GL_TEXTURE_2D_FORMAT_ARGB8888, width, height);
  z11_render_element_commit(render_element);

  struct timeval base, now;
  gettimeofday(&base, NULL);

  int ret;
  while (wl_display_dispatch_pending(global->display) != -1) {
    ret = wl_display_flush(global->display);
    if (ret == -1) break;
    gettimeofday(&now, NULL);
    if ((now.tv_sec - base.tv_sec) * 1000000 + now.tv_usec - base.tv_usec > 16666) {  // 60 hz
      // TODO: Enable to sync with compositor cycle
      theta += del_theta;
      if (theta >= 2 * M_PI || theta <= -2 * M_PI) theta = 0;
      base = now;
      paint_vertex(triangle_data, width, height, x, y, z, theta);
      z11_gl_vertex_buffer_allocate(vertex_buffer, size_of_triangles, triangle_buffer);
    }
  }
}

const char *vertex_shader =
    "#version 410\n"
    "uniform mat4 matrix;\n"
    "layout(location = 0) in vec4 position;\n"
    "layout(location = 1) in vec2 v2UVcoordsIn;\n"
    "layout(location = 2) in vec3 v3NormalIn;\n"
    "out vec2 v2UVcoords;\n"
    "void main()\n"
    "{\n"
    "  v2UVcoords = v2UVcoordsIn;\n"
    "  gl_Position = matrix * position;\n"
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

static void paint_vertex(Triangle *triangles, uint32_t width, uint32_t height, int x, int y, int z,
                         float theta)
{
  float h = height / 80;
  double root2x5 = sqrt(2) * width / 80;
  double root2x5_cos = root2x5 * cos(theta + M_PI / 4);
  double root2x5_sin = root2x5 * sin(theta + M_PI / 4);
  Vertex A = {{x - root2x5_cos, y - h, z - root2x5_sin}, {0, 1}};
  Vertex B = {{x + root2x5_sin, y - h, z - root2x5_cos}, {1, 1}};
  Vertex C = {{x + root2x5_sin, y + h, z - root2x5_cos}, {1, 0}};
  Vertex D = {{x - root2x5_cos, y + h, z - root2x5_sin}, {0, 0}};

  triangles->v1 = A;
  triangles->v2 = B;
  triangles->v3 = C;
  triangles++;
  triangles->v1 = A;
  triangles->v2 = C;
  triangles->v3 = D;
}

static void paint_texture(ColorBGRA *texture, uint8_t *png_data, uint32_t width, uint32_t height, uint32_t ch)
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
    fprintf(stderr, "Invalid png format\n");
    exit(1);
  }
}
