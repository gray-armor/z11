#define _GNU_SOURCE 1
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include "helper.h"

#include <assert.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>

#include "z11-client-protocol.h"
#include "z11-opengl-client-protocol.h"

void shm_format(void *data, struct wl_shm *wl_shm, uint32_t format) {}

struct wl_shm_listener shm_listener = {
    shm_format,
};

static void global_registry_handler(void *data, struct wl_registry *registry, uint32_t id,
                                    const char *interface, uint32_t version)
{
  struct z11_global *global = data;

  if (strcmp(interface, "z11_compositor") == 0) {
    global->compositor = wl_registry_bind(registry, id, &z11_compositor_interface, version);
  } else if (strcmp(interface, "wl_shm") == 0) {
    global->shm = wl_registry_bind(registry, id, &wl_shm_interface, 1);
    wl_shm_add_listener(global->shm, &shm_listener, NULL);
  } else if (strcmp(interface, "z11_opengl") == 0) {
    global->gl = wl_registry_bind(registry, id, &z11_opengl_interface, 1);
  } else if (strcmp(interface, "z11_opengl_render_component_manager") == 0) {
    global->render_component_manager =
        wl_registry_bind(registry, id, &z11_opengl_render_component_manager_interface, 1);
  }
}

static void global_registry_remover(void *data, struct wl_registry *registry, uint32_t id) {}

static const struct wl_registry_listener registry_listener = {
    global_registry_handler,
    global_registry_remover,
};

struct z11_global *z_helper_global()
{
  struct z11_global *global;
  struct wl_display *display;
  struct wl_registry *registry;

  global = malloc(sizeof *global);
  if (global == NULL) {
    fprintf(stderr, "Fail to allocate memory\n");
    goto no_mem_global;
  }

  global->compositor = NULL;
  global->shm = NULL;
  global->gl = NULL;
  global->render_component_manager = NULL;

  display = wl_display_connect(NULL);
  if (display == NULL) {
    fprintf(stderr, "Can't connect to display\n");
    goto fail_to_connect;
  }
  global->display = display;
  fprintf(stderr, "Connected to display\n");

  registry = wl_display_get_registry(display);

  wl_registry_add_listener(registry, &registry_listener, global);

  wl_display_dispatch(display);
  wl_display_roundtrip(display);

  assert(global->compositor && global->gl && global->shm && global->render_component_manager);

  return global;

fail_to_connect:
  free(global);

no_mem_global:
  return NULL;
}

int create_shared_fd(off_t size)
{
  char name[1024] = "";

  int fd = memfd_create("z11-simple-box", MFD_CLOEXEC | MFD_ALLOW_SEALING);
  if (fd < 0) {
    fprintf(stderr, "File cannot open: %s\n", name);
    exit(1);
  } else {
    unlink(name);
  }

  if (ftruncate(fd, size) < 0) {
    fprintf(stderr, "ftruncate failed: fd=%i, size=%li\n", fd, size);
    close(fd);
    exit(1);
  }

  return fd;
}

#define SIGNATURE_NUM 8

/**
 * TODO: transform variable PNG format to uniformed ARGB little format
 */
unsigned char *z_helper_png(const char *filename, __uint32_t *width, __uint32_t *height, __uint32_t *ch)
{
  FILE *fp;
  __uint32_t read_size;

  unsigned char **rows = NULL;
  unsigned char *data = NULL;
  png_struct *png;
  png_info *info;
  png_byte type, depth, compression, interlace, filter;
  png_byte signature[8];

  fp = fopen(filename, "rb");
  if (!fp) {
    fprintf(stderr, "Fail to open file: %s\n", filename);
    goto error_open_file;
  }

  read_size = fread(signature, 1, SIGNATURE_NUM, fp);

  if (png_sig_cmp(signature, 0, SIGNATURE_NUM)) {
    fprintf(stderr, "File is not png format.\n");
    goto error_not_png;
  }

  png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png == NULL) {
    fprintf(stderr, "Fail to crate png struct.\n");
    goto error_create_png_struct;
  }

  info = png_create_info_struct(png);
  if (info == NULL) {
    fprintf(stderr, "Fail to crate png image info.\n");
    goto error_create_info;
  }

  png_init_io(png, fp);
  png_set_sig_bytes(png, read_size);
  png_read_png(png, info, PNG_TRANSFORM_PACKING | PNG_TRANSFORM_STRIP_16, NULL);

  *width = png_get_image_width(png, info);
  *height = png_get_image_height(png, info);
  depth = png_get_bit_depth(png, info);
  type = png_get_color_type(png, info);
  compression = png_get_compression_type(png, info);
  interlace = png_get_interlace_type(png, info);
  filter = png_get_filter_type(png, info);

  // only support 8bit color depth image
  if (depth != 8) {
    fprintf(stderr, "Unsupported PNG format. We support only 8 bit color depth.\n");
    goto error_invalid_png_format;
  }

  if (compression != PNG_COMPRESSION_TYPE_BASE) {
    fprintf(stderr, "Unsupported PNG format.\n");
    goto error_invalid_png_format;
  }
  if (interlace != PNG_INTERLACE_NONE) {
    fprintf(stderr, "Unsupported PNG format.\n");
    goto error_invalid_png_format;
  }
  if (filter != PNG_FILTER_TYPE_BASE) {
    fprintf(stderr, "Unsupported PNG format.\n");
    goto error_invalid_png_format;
  }

  // only RGB and RGBA support for now
  if (type == PNG_COLOR_TYPE_RGB)
    *ch = 3;
  else if (type == PNG_COLOR_TYPE_RGB_ALPHA)
    *ch = 4;
  else {
    fprintf(stderr, "Unsupported PNG format. We support only RGB and RGBA format. (%u)\n", type);
    goto error_invalid_png_format;
  }

  rows = png_get_rows(png, info);
  data = malloc(sizeof(*data) * (*width) * (*height) * (*ch));
  if (data == NULL) goto err_malloc_return_val;

  for (uint32_t i = 0; i < (*height); i++) {
    memcpy(data + i * (*width) * (*ch), rows[i], (*width) * (*ch));
  }

err_malloc_return_val:
error_invalid_png_format:
error_create_info:
  png_destroy_read_struct(&png, &info, NULL);

error_create_png_struct:
error_not_png:
  fclose(fp);

error_open_file:
  return data;
}

size_t HEADER_BYTE_SIZE = 80;
size_t FACE_INFO_BYTE_SIZE = 4;
size_t FACET_SIZE = 50;

static void set_point_from_stl(Point *p, char *facet)
{
  char f1[4] = {facet[0], facet[1], facet[2], facet[3]};
  char f2[4] = {facet[4], facet[5], facet[6], facet[7]};
  char f3[4] = {facet[8], facet[9], facet[10], facet[11]};
  p->x = *((float *)f1);
  p->y = *((float *)f2);
  p->z = *((float *)f3);
}

Face *z_helper_stl(const char *filename, int *face_count)
{
  FILE *fp;
  char header_info[80];
  Face *data = NULL;

  fp = fopen(filename, "rb");
  if (!fp) {
    fprintf(stderr, "Fail to open file: %s\n", filename);
    goto error_open_file;
  }

  if (HEADER_BYTE_SIZE != fread(header_info, sizeof(char), HEADER_BYTE_SIZE, fp)) {
    fprintf(stderr, "Error reading %s\n", filename);
    goto error_read_file;
  }
  fprintf(stderr, "header: %s\n", header_info);

  if (FACE_INFO_BYTE_SIZE / sizeof(int) !=
      fread(face_count, sizeof(int), FACE_INFO_BYTE_SIZE / sizeof(int), fp)) {
    fprintf(stderr, "Error reading %s\n", filename);
    goto error_read_file;
  }
  fprintf(stderr, "face count: %d\n", *face_count);

  data = (Face *)malloc(sizeof(Face) * *face_count);

  Point *p = malloc(sizeof(*p));
  for (int i = 0; i < (int)*face_count; i++) {
    char facet[FACET_SIZE];

    if (FACET_SIZE != fread(facet, sizeof(char), FACET_SIZE, fp)) {
      fprintf(stderr, "Error reading %s\n", filename);
      free(p);
      goto error_read_file;
    }

    set_point_from_stl(p, facet + 12);
    data->p1 = *p;
    set_point_from_stl(p, facet + 24);
    data->p2 = *p;
    set_point_from_stl(p, facet + 36);
    data->p3 = *p;
    data++;
  }
  free(p);
  data -= *face_count;

error_read_file:
  fclose(fp);

error_open_file:
  return data;
}
