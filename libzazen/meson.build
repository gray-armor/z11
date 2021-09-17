deps_libzazen = [
  wayland_server_dep,
  dependency('glew'),
  dependency('libinput'),
  dependency('libudev'),
]

srcs_libzazen = files([
  'callback.c',
  'compositor.c',
  'cuboid_window.c',
  'input.c',
  'keyboard.c',
  'lib_input.c',
  'opengl_render_component_manager.c',
  'opengl_render_component.c',
  'opengl_render_item.c',
  'opengl_shader_program.c',
  'opengl_texture_2d.c',
  'opengl_render_component_back_state.c',
  'opengl_vertex_buffer.c',
  'opengl.c',
  'ray.c',
  'seat.c',
  'shell.c',
  'util.c',
  'virtual_object.c',
]) + [
  z11_protocol_c,
  z11_server_protocol_h,
  z11_input_protocol_c,
  z11_input_server_protocol_h,
  z11_opengl_protocol_c,
  z11_opengl_server_protocol_h,
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
