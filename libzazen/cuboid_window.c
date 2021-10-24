#include "cuboid_window.h"

#include <libzazen.h>
#include <wayland-server.h>
#include <z11-server-protocol.h>

#include "opengl_render_component_manager.h"
#include "ray.h"
#include "shell.h"
#include "util.h"
#include "virtual_object.h"

static const char* fragment_shader;
static const char* vertex_shader;
static void zazen_cuboid_window_update_vertex_buffer(
    struct zazen_cuboid_window* cuboid_window, float l);

static void zazen_cuboid_window_destroy(
    struct zazen_cuboid_window* cuboid_window);

static void zazen_cuboid_window_handle_destroy(struct wl_resource* resource)
{
  struct zazen_cuboid_window* cuboid_window;

  cuboid_window = wl_resource_get_user_data(resource);

  zazen_cuboid_window_destroy(cuboid_window);
}

static void zazen_cuboid_window_protocol_destroy(struct wl_client* client,
                                                 struct wl_resource* resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void zazen_cuboid_window_protocol_request_window_size(
    struct wl_client* client, struct wl_resource* resource, wl_fixed_t width,
    wl_fixed_t height, wl_fixed_t depth)
{
  UNUSED(client);
  struct zazen_cuboid_window* cuboid_window;

  cuboid_window = wl_resource_get_user_data(resource);

  cuboid_window->width = wl_fixed_to_double(width);
  cuboid_window->height = wl_fixed_to_double(height);
  cuboid_window->depth = wl_fixed_to_double(depth);

  z11_cuboid_window_send_configure(resource, width, height, depth);

  zazen_cuboid_window_update_vertex_buffer(cuboid_window, 0.25);
  zazen_opengl_render_item_set_vertex_buffer(
      cuboid_window->render_item, cuboid_window->vertex_buffer,
      sizeof cuboid_window->vertex_buffer, sizeof(vec3));
  zazen_opengl_render_item_commit(cuboid_window->render_item);
}

static const struct z11_cuboid_window_interface zazen_cuboid_window_interface =
    {
        .destroy = zazen_cuboid_window_protocol_destroy,
        .request_window_size = zazen_cuboid_window_protocol_request_window_size,
};

static void virtual_object_model_matrix_change_handler(
    struct wl_listener* listener, void* data)
{
  struct zazen_virtual_object* virtual_object = data;
  struct zazen_cuboid_window* cuboid_window;

  cuboid_window = wl_container_of(listener, cuboid_window,
                                  virtual_object_model_matrix_change_listener);

  zazen_opengl_render_item_set_model_matrix(cuboid_window->render_item,
                                            virtual_object->model_matrix);
}

static void virtual_object_destroy_handler(struct wl_listener* listener,
                                           void* data)
{
  UNUSED(data);
  struct zazen_cuboid_window* cuboid_window;

  cuboid_window =
      wl_container_of(listener, cuboid_window, virtual_object_destroy_listener);

  wl_resource_set_destructor(cuboid_window->resource, NULL);
  wl_resource_set_user_data(cuboid_window->resource, NULL);
  zazen_cuboid_window_destroy(cuboid_window);
  wl_resource_post_error(
      cuboid_window->resource, WL_DISPLAY_ERROR_IMPLEMENTATION,
      "cuboid window must be destroyed before its virtual object.");
}

void zazen_cuboid_window_highlight(struct zazen_cuboid_window* cuboid_window)
{
  zazen_cuboid_window_update_vertex_buffer(cuboid_window, 0.5);
  zazen_opengl_render_item_set_vertex_buffer(
      cuboid_window->render_item, cuboid_window->vertex_buffer,
      sizeof cuboid_window->vertex_buffer, sizeof(vec3));
  zazen_opengl_render_item_commit(cuboid_window->render_item);
}

void zazen_cuboid_window_remove_highlight(
    struct zazen_cuboid_window* cuboid_window)
{
  zazen_cuboid_window_update_vertex_buffer(cuboid_window, 0.25);
  zazen_opengl_render_item_set_vertex_buffer(
      cuboid_window->render_item, cuboid_window->vertex_buffer,
      sizeof cuboid_window->vertex_buffer, sizeof(vec3));
  zazen_opengl_render_item_commit(cuboid_window->render_item);
}

struct zazen_cuboid_window* zazen_cuboid_window_create(
    struct wl_client* client, uint32_t id,
    struct zazen_virtual_object* virtual_object, struct zazen_shell* shell,
    struct zazen_opengl_render_component_manager* manager)
{
  struct zazen_cuboid_window* cuboid_window;
  struct wl_resource* resource;

  cuboid_window = zalloc(sizeof *cuboid_window);
  if (cuboid_window == NULL) {
    wl_client_post_no_memory(client);
    goto out;
  }

  cuboid_window->render_item = zazen_opengl_render_item_create(manager);
  if (cuboid_window->render_item == NULL) {
    wl_client_post_no_memory(client);
    goto out_cuboid_window;
  }

  resource = wl_resource_create(client, &z11_cuboid_window_interface, 1, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    goto out_render_item;
  }

  wl_resource_set_implementation(resource, &zazen_cuboid_window_interface,
                                 cuboid_window,
                                 zazen_cuboid_window_handle_destroy);

  cuboid_window->resource = resource;

  cuboid_window->virtual_object = virtual_object;
  cuboid_window->virtual_object_destroy_listener.notify =
      virtual_object_destroy_handler;
  wl_signal_add(&virtual_object->destroy_signal,
                &cuboid_window->virtual_object_destroy_listener);

  cuboid_window->virtual_object_model_matrix_change_listener.notify =
      virtual_object_model_matrix_change_handler;
  wl_signal_add(&virtual_object->model_matrix_change_signal,
                &cuboid_window->virtual_object_model_matrix_change_listener);

  zazen_opengl_render_item_set_shader(cuboid_window->render_item, vertex_shader,
                                      fragment_shader);
  zazen_opengl_render_item_set_topology(cuboid_window->render_item,
                                        Z11_OPENGL_TOPOLOGY_LINES);
  zazen_opengl_render_item_append_vertex_input_attribute(
      cuboid_window->render_item, 0,
      Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR3, 0);
  zazen_opengl_render_item_set_model_matrix(cuboid_window->render_item,
                                            virtual_object->model_matrix);

  wl_signal_init(&cuboid_window->destroy_signal);
  wl_list_insert(&shell->cuboid_window_list, &cuboid_window->link);

  return cuboid_window;

out_render_item:
  zazen_opengl_render_item_destroy(cuboid_window->render_item);

out_cuboid_window:
  free(cuboid_window);

out:
  return NULL;
}

static void zazen_cuboid_window_destroy(
    struct zazen_cuboid_window* cuboid_window)
{
  wl_signal_emit(&cuboid_window->destroy_signal, NULL);
  zazen_opengl_render_item_destroy(cuboid_window->render_item);
  wl_list_remove(&cuboid_window->virtual_object_destroy_listener.link);
  wl_list_remove(&cuboid_window->link);
  free(cuboid_window);
}

static const char* vertex_shader =
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

static const char* fragment_shader =
    "#version 410 core\n"
    "in vec2 v2UVcoords;\n"
    "out vec4 outputColor;\n"
    "void main()\n"
    "{\n"
    "  outputColor = vec4(0.0, 0.0, 0.543, 1.0);\n"
    "}\n";

static void zazen_cuboid_window_update_vertex_buffer(
    struct zazen_cuboid_window* cuboid_window, float frame_length)
{
  float w = cuboid_window->width / 2;
  float h = cuboid_window->height / 2;
  float d = cuboid_window->depth / 2;
  float l = 1 - frame_length;
  vec3 A = {-w, -h, -d};
  vec3 B = {+w, -h, -d};
  vec3 C = {+w, +h, -d};
  vec3 D = {-w, +h, -d};
  vec3 E = {-w, -h, +d};
  vec3 F = {+w, -h, +d};
  vec3 G = {+w, +h, +d};
  vec3 H = {-w, +h, +d};
  vec3 AB = {-w * l, -h, -d};
  vec3 BA = {+w * l, -h, -d};
  vec3 BC = {+w, -h * l, -d};
  vec3 CB = {+w, +h * l, -d};
  vec3 CD = {+w * l, +h, -d};
  vec3 DC = {-w * l, +h, -d};
  vec3 DA = {-w, +h * l, -d};
  vec3 AD = {-w, -h * l, -d};
  vec3 EF = {-w * l, -h, +d};
  vec3 FE = {+w * l, -h, +d};
  vec3 FG = {+w, -h * l, +d};
  vec3 GF = {+w, +h * l, +d};
  vec3 GH = {+w * l, +h, +d};
  vec3 HG = {-w * l, +h, +d};
  vec3 HE = {-w, +h * l, +d};
  vec3 EH = {-w, -h * l, +d};
  vec3 AE = {-w, -h, -d * l};
  vec3 EA = {-w, -h, +d * l};
  vec3 BF = {+w, -h, -d * l};
  vec3 FB = {+w, -h, +d * l};
  vec3 CG = {+w, +h, -d * l};
  vec3 GC = {+w, +h, +d * l};
  vec3 DH = {-w, +h, -d * l};
  vec3 HD = {-w, +h, +d * l};
  glm_vec3_copy(A, cuboid_window->vertex_buffer[0]);
  glm_vec3_copy(AB, cuboid_window->vertex_buffer[1]);
  glm_vec3_copy(BA, cuboid_window->vertex_buffer[2]);
  glm_vec3_copy(B, cuboid_window->vertex_buffer[3]);
  glm_vec3_copy(B, cuboid_window->vertex_buffer[4]);
  glm_vec3_copy(BC, cuboid_window->vertex_buffer[5]);
  glm_vec3_copy(CB, cuboid_window->vertex_buffer[6]);
  glm_vec3_copy(C, cuboid_window->vertex_buffer[7]);
  glm_vec3_copy(C, cuboid_window->vertex_buffer[8]);
  glm_vec3_copy(CD, cuboid_window->vertex_buffer[9]);
  glm_vec3_copy(DC, cuboid_window->vertex_buffer[10]);
  glm_vec3_copy(D, cuboid_window->vertex_buffer[11]);
  glm_vec3_copy(D, cuboid_window->vertex_buffer[12]);
  glm_vec3_copy(DA, cuboid_window->vertex_buffer[13]);
  glm_vec3_copy(AD, cuboid_window->vertex_buffer[14]);
  glm_vec3_copy(A, cuboid_window->vertex_buffer[15]);
  glm_vec3_copy(E, cuboid_window->vertex_buffer[16]);
  glm_vec3_copy(EF, cuboid_window->vertex_buffer[17]);
  glm_vec3_copy(FE, cuboid_window->vertex_buffer[18]);
  glm_vec3_copy(F, cuboid_window->vertex_buffer[19]);
  glm_vec3_copy(F, cuboid_window->vertex_buffer[20]);
  glm_vec3_copy(FG, cuboid_window->vertex_buffer[21]);
  glm_vec3_copy(GF, cuboid_window->vertex_buffer[22]);
  glm_vec3_copy(G, cuboid_window->vertex_buffer[23]);
  glm_vec3_copy(G, cuboid_window->vertex_buffer[24]);
  glm_vec3_copy(GH, cuboid_window->vertex_buffer[25]);
  glm_vec3_copy(HG, cuboid_window->vertex_buffer[26]);
  glm_vec3_copy(H, cuboid_window->vertex_buffer[27]);
  glm_vec3_copy(H, cuboid_window->vertex_buffer[28]);
  glm_vec3_copy(HE, cuboid_window->vertex_buffer[29]);
  glm_vec3_copy(EH, cuboid_window->vertex_buffer[30]);
  glm_vec3_copy(E, cuboid_window->vertex_buffer[31]);
  glm_vec3_copy(A, cuboid_window->vertex_buffer[32]);
  glm_vec3_copy(AE, cuboid_window->vertex_buffer[33]);
  glm_vec3_copy(EA, cuboid_window->vertex_buffer[34]);
  glm_vec3_copy(E, cuboid_window->vertex_buffer[35]);
  glm_vec3_copy(B, cuboid_window->vertex_buffer[36]);
  glm_vec3_copy(BF, cuboid_window->vertex_buffer[37]);
  glm_vec3_copy(FB, cuboid_window->vertex_buffer[38]);
  glm_vec3_copy(F, cuboid_window->vertex_buffer[39]);
  glm_vec3_copy(C, cuboid_window->vertex_buffer[40]);
  glm_vec3_copy(CG, cuboid_window->vertex_buffer[41]);
  glm_vec3_copy(GC, cuboid_window->vertex_buffer[42]);
  glm_vec3_copy(G, cuboid_window->vertex_buffer[43]);
  glm_vec3_copy(D, cuboid_window->vertex_buffer[44]);
  glm_vec3_copy(DH, cuboid_window->vertex_buffer[45]);
  glm_vec3_copy(HD, cuboid_window->vertex_buffer[46]);
  glm_vec3_copy(H, cuboid_window->vertex_buffer[47]);
}

//                                         /z+
//                                        /
//                    H.--------.HG  GH.---------.G
//                    /|                /       /|
//                   / |        ^y     /       / |
//                HD.  |        |     /     GC.  |
//                     .HE      |    /           .GF
//              DH.             |   /       .CG
//               /     .EH      |  /       /     .FG
//              /      |        | /       /      |
//            D.---------.DC CD.---------.C      |
//  -----------|--------------- / -------|------------>x
//             |      E.-------/-.EF FE.-|-------.F
//             |      /       / |        |      /
//           DA.     /       /  |      CB.     /
//                  .EA     /   |             .FB
//           AD.           /    |      BC.
//             |  .AE     /     |        |  .BF
//             | /       /      |        | /
//             |/               |        |/
//             .---------.     .---------.
//            A          AB   BA|        B
//                              |
//                              |
