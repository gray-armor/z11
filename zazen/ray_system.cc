#include "ray_system.h"

#include <float.h>
#include <libzazen.h>

#include "matrices.h"
#include "vectors.h"

static float RayOBBIntersection(Vector3 ray_origin, Vector3 ray_direction,
                                Vector3 aabb_min, Vector3 aabb_max,
                                float model_matrix[16]);

void RaySystem::CalculateInterection(
    struct zazen_ray_back_state *ray_back_state,
    ZServer::CuboidWindowIterator *cuboid_window_iterator)
{
  struct zazen_cuboid_window_back_state *cuboid_window_state;
  struct zazen_cuboid_window *focus_cuboid_window = nullptr;
  float min_distance = FLT_MAX;

  Vector3 ray_origin(ray_back_state->origin[0], ray_back_state->origin[1],
                     ray_back_state->origin[2]);
  Vector3 ray_direction(ray_back_state->direction[0],
                        ray_back_state->direction[1],
                        ray_back_state->direction[2]);

  while ((cuboid_window_state = cuboid_window_iterator->Next())) {
    float w = cuboid_window_state->width / 2;
    float h = cuboid_window_state->height / 2;
    float d = cuboid_window_state->depth / 2;
    Vector3 aabb_min(-w, -h, -d);
    Vector3 aabb_max(w, h, d);
    float distance =
        RayOBBIntersection(ray_origin, ray_direction, aabb_min, aabb_max,
                           cuboid_window_state->model_matrix);
    if (distance >= 0 && distance < min_distance) {
      min_distance = d;
      focus_cuboid_window = cuboid_window_state->cuboid_window;
    }
  }

  if (focus_cuboid_window != nullptr)
    zazen_ray_enter(ray_back_state->ray, focus_cuboid_window);
}

// this function returns the distance between the ray origin point and the
// nearest intersection point of the ray and the OBB. If not intersected, this
// returns the negative value.
static float RayOBBIntersection(Vector3 ray_origin, Vector3 ray_direction,
                                Vector3 aabb_min, Vector3 aabb_max,
                                float model_matrix[16])
{
  float t_min = 0.0f;
  float t_max = FLT_MAX;

  Vector3 obb_position_worldspace(model_matrix[12], model_matrix[13],
                                  model_matrix[14]);

  Vector3 delta = obb_position_worldspace - ray_origin;

  // X axis
  {
    Vector3 axis(model_matrix[0], model_matrix[1], model_matrix[2]);
    float e = axis.dot(delta);
    float f = ray_direction.dot(axis);

    if (fabs(f) > 0.001f) {
      float t1 = (e + aabb_min[0]) / f;
      float t2 = (e + aabb_max[0]) / f;
      if (t1 > t2) {
        float w = t1;
        t1 = t2;
        t2 = w;
      }

      if (t2 < t_max) t_max = t2;
      if (t1 > t_min) t_min = t1;
      if (t_max < t_min) return -1;
    } else {
      if (-e + aabb_min[0] > 0.0f || -e + aabb_max[0] < 0.0f) return -1;
    }
  }

  // Y axis
  {
    Vector3 axis(model_matrix[4], model_matrix[5], model_matrix[6]);
    float e = axis.dot(delta);
    float f = ray_direction.dot(axis);

    if (fabs(f) > 0.001f) {
      float t1 = (e + aabb_min[1]) / f;
      float t2 = (e + aabb_max[1]) / f;
      if (t1 > t2) {
        float w = t1;
        t1 = t2;
        t2 = w;
      }

      if (t2 < t_max) t_max = t2;
      if (t1 > t_min) t_min = t1;
      if (t_max < t_min) return -1;
    } else {
      if (-e + aabb_min[1] > 0.0f || -e + aabb_max[1] < 0.0f) return -1;
    }

    // Z axis
    {
      Vector3 axis(model_matrix[8], model_matrix[9], model_matrix[10]);
      float e = axis.dot(delta);
      float f = ray_direction.dot(axis);

      if (fabs(f) > 0.001f) {
        float t1 = (e + aabb_min[2]) / f;
        float t2 = (e + aabb_max[2]) / f;
        if (t1 > t2) {
          float w = t1;
          t1 = t2;
          t2 = w;
        }

        if (t2 < t_max) t_max = t2;
        if (t1 > t_min) t_min = t1;
        if (t_max < t_min) return -1;
      } else {
        if (-e + aabb_min[2] > 0.0f || -e + aabb_max[2] < 0.0f) return -1;
      }
    }

    return t_min;
  }
}
