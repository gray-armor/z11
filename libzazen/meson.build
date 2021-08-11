deps_libzazen = [
  wayland_server_dep,
  dependency('glew'),
]

srcs_libzazen = files([
  'compositor.c',
  'gl.c',
  'gl_shader_program.c',
  'gl_texture_2d.c',
  'gl_vertex_buffer.c',
  'render_element.c',
  'render_element_state.c',
  'util.c',
]) + [
  z11_protocol_c,
  z11_server_protocol_h,
]

lib_zazen = static_library(
  'zazen',
  srcs_libzazen,
  include_directories : public_inc,
  version: '0.0.@0@'.format(libzazen_revision),
  dependencies : deps_libzazen,
)

deps_for_libzazen_users = [
  wayland_server_dep,
]

libzazen_public_dep = declare_dependency(
  link_with : lib_zazen,
  include_directories : public_inc,
  dependencies : deps_for_libzazen_users,
)
