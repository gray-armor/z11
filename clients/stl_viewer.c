#pragma GCC diagnostic ignored "-Wunused-variable"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>

#include "helper.h"
#include "z11-client-protocol.h"

typedef struct {
  Point point;
} Vertex;

typedef struct {
  Vertex v1, v2, v3;
} Triangle;

const char *vertex_shader;
const char *fragment_shader;

void set_pointer(Point *p, char *facet)
{
  char f1[4] = {facet[0], facet[1], facet[2], facet[3]};
  char f2[4] = {facet[4], facet[5], facet[6], facet[7]};
  char f3[4] = {facet[8], facet[9], facet[10], facet[11]};
  p->x = *((float *)f1);
  p->y = *((float *)f2);
  p->z = *((float *)f3);
}

int main(int argc, char const *argv[])
{
  const char *filename;
  char *data = NULL;

  filename = argv[1];

  int dx = 0;
  int dy = 0;
  int dz = 20;
  if (argc > 2) dx = atoi(argv[2]);
  if (argc > 3) dy = atoi(argv[3]);
  if (argc > 4) dz = atoi(argv[4]);

  FILE *fp;
  char header_info[80];
  int face_count;

  char *t;
  fp = fopen(filename, "rb");
  if (!fp) {
    fprintf(stderr, "Fail to open file: %s\n", filename);
    return 0;
  }

  struct z11_global *global = z_helper_global();

  size_t size = fread(header_info, sizeof(char), 80, fp);
  fprintf(stderr, "header: %s\n", header_info);
  fprintf(stderr, "size: %ld\n", size);
  size_t size2 = fread(&face_count, sizeof(int), 1, fp);
  fprintf(stderr, "face count: %d\n", face_count);
  fprintf(stderr, "size: %ld\n", size2);

  int size_of_triangles = sizeof(Triangle) * face_count * 2;  // TODO
  int mem_size = size_of_triangles;
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

  Triangle *triangles = (Triangle *)shm_data;

  for (int i = 0; i < face_count; i++) {
    char facet[50];

    size_t size = fread(facet, sizeof(char), 50, fp);
    (void)size;
    Point *p = malloc(sizeof(*p));
    set_pointer(p, facet + 12);
    Vertex A = {{p->x + dx, p->y + dy, p->z + dz}};
    set_pointer(p, facet + 24);
    Vertex B = {{p->x + dx, p->y + dy, p->z + dz}};
    set_pointer(p, facet + 36);
    Vertex C = {{p->x + dx, p->y + dy, p->z + dz}};
    triangles->v1 = A;
    triangles->v2 = B;
    triangles->v3 = C;
    triangles++;
    free(p);
  }

  fclose(fp);

  struct wl_shm_pool *pool = wl_shm_create_pool(global->shm, fd, size_of_triangles);
  struct wl_raw_buffer *triangle_buffer = wl_shm_pool_create_raw_buffer(pool, 0, size_of_triangles);
  wl_shm_pool_destroy(pool);

  struct z11_gl_vertex_buffer *triangle_vertex_buffer = z11_gl_create_vertex_buffer(global->gl);

  struct z11_gl_shader_program *frame_shader_program =
      z11_gl_create_shader_program(global->gl, vertex_shader, fragment_shader);

  struct z11_render_block *frame_render_block = z11_compositor_create_render_block(global->compositor);
  z11_render_block_attach_vertex_buffer(frame_render_block, triangle_vertex_buffer, sizeof(Point));
  z11_render_block_attach_shader_program(frame_render_block, frame_shader_program);
  z11_render_block_append_vertex_input_attribute(  //
      frame_render_block, 0, Z11_GL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR3, 0);

  z11_render_block_set_topology(frame_render_block, Z11_GL_TOPOLOGY_LINES);

  z11_gl_vertex_buffer_allocate(triangle_vertex_buffer, size_of_triangles, triangle_buffer);
  z11_render_block_commit(frame_render_block);

  int ret;
  while (wl_display_dispatch_pending(global->display) != -1) {
    ret = wl_display_flush(global->display);
    if (ret == -1) break;
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
    "in vec2 v2UVcoords;\n"
    "out vec4 outputColor;\n"
    "void main()\n"
    "{\n"
    "  outputColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
    "}\n";
