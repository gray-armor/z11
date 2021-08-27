#include <wayland-server-core.h>

#include "compositor.h"
#include "libinput_device.h"
#include "opengl_render_component_manager.h"

int zazen_input_init(struct wl_event_loop* loop,
                     struct zazen_opengl_render_component_manager* render_component_managerstruct)
{
  libinput_init(loop);  // TODO: error handling
  return 1;
}

void zazen_input_destory() { libinput_destroy(); }
