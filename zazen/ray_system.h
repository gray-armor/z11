#ifndef ZAZEN_RAY_SYSTEM_H
#define ZAZEN_RAY_SYSTEM_H

#include "z_server.h"

class RaySystem
{
 public:
  void CalculateInterection(
      struct zazen_ray_back_state *ray_back_state,
      ZServer::CuboidWindowIterator *cuboid_window_iterator);
};

#endif  //  ZAZEN_RAY_SYSTEM_H
