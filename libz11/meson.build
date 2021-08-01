deps_libz11 = [
  wayland_server_dep,
  dependency('glew'),
]

srcs_libz11 = files([
  'compositor.c',
  'gl.c',
  'gl_shader_program.c',
  'gl_vertex_buffer.c',
  'render_block.c',
  'render_block_state.c',
]) + [
  z11_protocol_c,
  z11_server_protocol_h,
]

lib_z11 = shared_library(
  'z11',
  srcs_libz11,
  include_directories : public_inc,
  install : true,
  version: '0.0.@0@'.format(libz11_revision),
  dependencies : deps_libz11,
)

deps_for_libz11_users = [
  wayland_server_dep,
]

libz11_public_dep = declare_dependency(
  link_with : lib_z11,
  include_directories : public_inc,
  dependencies : deps_for_libz11_users,
)
