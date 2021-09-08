#include "input.h"

#include "seat.h"
#include "util.h"

struct zazen_input *zazen_input_create(struct wl_event_loop *loop,
                                       struct zazen_opengl_render_component_manager *render_component_manager)
{
  struct zazen_input *input;

  input = zalloc(sizeof *input);

  input->libinput = zazen_libinput_create(loop, render_component_manager);
  if (!input->libinput) {
    zazen_log("Unable to initialize libinput\n");
    goto out;
  }

  return input;

out:
  free(input);

  return NULL;
}

void zazen_input_destroy(struct zazen_input *input)
{
  zazen_libinput_destroy(input->libinput);
  free(input);
}
