#ifndef LIBZAZEN_OPENGL_SHADER_PROGRAM_H
#define LIBZAZEN_OPENGL_SHADER_PROGRAM_H

#include <wayland-server.h>

struct zazen_opengl_shader_program {
  char* vertex_shader_source;
  char* fragment_shader_source;
  struct wl_signal destroy_signal;
  struct wl_signal state_change_signal;
};

struct zazen_opengl_shader_program* zazen_opengl_shader_program_create(struct wl_client* client, uint32_t id,
                                                                       const char* vertex_shader_source,
                                                                       const char* fragment_shader_source);

#endif  //  LIBZAZEN_OPENGL_SHADER_PROGRAM_H
