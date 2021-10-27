#include "compositor.h"

#include <float.h>

#include "cuboid_window.h"
#include "shell.h"
#include "util.h"
#include "virtual_object.h"
#include "z11-server-protocol.h"

static void zazen_compositor_protocol_create_virtual_object(
    struct wl_client* client, struct wl_resource* resource, uint32_t id)
{
  struct zazen_compositor* compositor;
  struct zazen_virtual_object* virtual_object;

  compositor = wl_resource_get_user_data(resource);

  virtual_object = zazen_virtual_object_create(client, id, compositor);
  if (virtual_object == NULL) {
    zazen_log("Failed to create a virtual object\n");
  }
}

static const struct z11_compositor_interface zazen_compositor_interface = {
    .create_virtual_object = zazen_compositor_protocol_create_virtual_object,
};

static void zazen_compositor_bind(struct wl_client* client, void* data,
                                  uint32_t version, uint32_t id)
{
  struct zazen_compositor* compositor = data;
  struct wl_resource* resource;

  resource = wl_resource_create(client, &z11_compositor_interface, version, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    return;
  }

  wl_resource_set_implementation(resource, &zazen_compositor_interface,
                                 compositor, NULL);
}

void zazen_compositor_emit_frame_signal(struct zazen_compositor* compositor)
{
  wl_signal_emit(&compositor->frame_signal, compositor);
}

struct zazen_compositor* zazen_compositor_create(struct wl_display* display)
{
  struct zazen_compositor* compositor;

  compositor = zalloc(sizeof *compositor);
  if (compositor == NULL) goto out;

  if (wl_global_create(display, &z11_compositor_interface, 1, compositor,
                       zazen_compositor_bind) == NULL)
    goto out;

  wl_signal_init(&compositor->frame_signal);

  return compositor;

out:
  return NULL;
}

static int ray_obb_intersection_axis_test(vec3 axis, vec3 translation,
                                          vec3 ray_direction, float axis_min,
                                          float axis_max, float* t_min,
                                          float* t_max)
{
  float e = glm_vec3_dot(axis, translation);
  float f = glm_vec3_dot(ray_direction, axis);

  if (fabs(f) > 0.001f) {
    float t1 = (e + axis_min) / f;
    float t2 = (e + axis_max) / f;
    if (t1 > t2) glm_swapf(&t1, &t2);
    if (t2 < *t_max) *t_max = t2;
    if (*t_min < t1) *t_min = t1;
    if (*t_max < *t_min) return -1;
  } else {
    if (-e + axis_min > 0.0f || -e + axis_max < 0.0f) return -1;
  }
  return 0;
}

static float ray_obb_intersection(vec3 ray_origin, vec3 ray_direction,
                                  vec3 aabb_min, vec3 aabb_max,
                                  mat4 model_matrix)
{
  float t_min = 0.0f;
  float t_max = FLT_MAX;

  vec3 obb_position_worldspace, translation;
  glm_vec4_copy3(model_matrix[3], obb_position_worldspace);
  glm_vec3_sub(obb_position_worldspace, ray_origin, translation);

  for (int i = 0; i < 3; i++) {  // test x, y and z axis
    vec3 axis;
    glm_vec4_copy3(model_matrix[i], axis);
    if (ray_obb_intersection_axis_test(axis, translation, ray_direction,
                                       aabb_min[i], aabb_max[i], &t_min,
                                       &t_max) == -1)
      return -1;
  }

  return t_min;
}

struct zazen_cuboid_window* zazen_compositor_pick_cuboid_window(
    struct zazen_compositor* compositor, vec3 ray_origin, vec3 ray_direction,
    vec3 local_ray_origin, vec3 local_ray_direction, float* distance)
{
  struct zazen_shell* shell = compositor->shell;
  struct zazen_cuboid_window* focus_cuboid_window = NULL;
  struct zazen_cuboid_window* cuboid_window;
  float min_distance = FLT_MAX;

  wl_list_for_each(cuboid_window, &shell->cuboid_window_list, link)
  {
    float x = cuboid_window->width / 2;
    float y = cuboid_window->height / 2;
    float z = cuboid_window->depth / 2;
    vec3 aabb_min = {-x, -y, -z};
    vec3 aabb_max = {+x, +y, +z};
    float d =
        ray_obb_intersection(ray_origin, ray_direction, aabb_min, aabb_max,
                             cuboid_window->virtual_object->model_matrix);
    if (d >= 0 && d < min_distance) {
      min_distance = d;
      focus_cuboid_window = cuboid_window;
    }
  }

  if (focus_cuboid_window) {
    mat4 model_matrix_inverse = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_inv(focus_cuboid_window->virtual_object->model_matrix,
                 model_matrix_inverse);
    vec3 ray_target, local_ray_target;
    glm_vec3_add(ray_origin, ray_direction, ray_target);
    glm_mat4_mulv3(model_matrix_inverse, ray_target, 1, local_ray_target);
    glm_mat4_mulv3(model_matrix_inverse, ray_origin, 1, local_ray_origin);
    glm_vec3_sub(local_ray_target, local_ray_origin, local_ray_direction);
    glm_vec3_normalize(local_ray_direction);
    *distance = min_distance;
  }

  return focus_cuboid_window;
}
