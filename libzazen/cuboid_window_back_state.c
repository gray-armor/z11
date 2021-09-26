#include "cuboid_window_back_state.h"

#include <cglm/cglm.h>
#include <string.h>

#include "shell.h"
#include "util.h"

struct zazen_cuboid_window_back_state *zazen_cuboid_window_back_state_create(
    struct zazen_virtual_object *virtual_object, struct zazen_shell *shell,
    struct zazen_cuboid_window *cuboid_window, float width, float height,
    float depth)
{
  struct zazen_cuboid_window_back_state *back_state;
  back_state = zalloc(sizeof *back_state);
  if (back_state == NULL) return NULL;

  back_state->width = width;
  back_state->height = height;
  back_state->depth = depth;
  back_state->cuboid_window = cuboid_window;
  memcpy(back_state->model_matrix, virtual_object->model_matrix,
         sizeof(float) * 16);

  wl_list_insert(&shell->cuboid_window_back_state_list, &back_state->link);

  return back_state;
}

void zazen_cuboid_window_back_state_destroy(
    struct zazen_cuboid_window_back_state *back_state)
{
  wl_list_remove(&back_state->link);
  free(back_state);
}

void zazen_cuboid_window_back_state_update(
    struct zazen_cuboid_window_back_state *back_state,
    struct zazen_virtual_object *virtual_object, float width, float height,
    float depth)
{
  back_state->width = width;
  back_state->height = height;
  back_state->depth = depth;
  memcpy(back_state->model_matrix, virtual_object->model_matrix,
         sizeof(float) * 16);
}
