#ifndef LIBZAZEN_CUBOID_WINDOW_BACK_STATE_H
#define LIBZAZEN_CUBOID_WINDOW_BACK_STATE_H

#include <libzazen.h>

#include "cuboid_window.h"
#include "shell.h"
#include "virtual_object.h"

struct zazen_cuboid_window_back_state *zazen_cuboid_window_back_state_create(
    struct zazen_virtual_object *virtual_object, struct zazen_shell *shell,
    struct zazen_cuboid_window *cuboid_window, float width, float height,
    float depth);

void zazen_cuboid_window_back_state_update(
    struct zazen_cuboid_window_back_state *back_state,
    struct zazen_virtual_object *virtual_object, float width, float height,
    float depth);

void zazen_cuboid_window_back_state_destroy(
    struct zazen_cuboid_window_back_state *back_state);

#endif  //  LIBZAZEN_CUBOID_WINDOW_BACK_STATE_H
