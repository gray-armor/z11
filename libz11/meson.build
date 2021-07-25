deps_libz11 = [
  wayland_server_dep,
  dependency('glew'),
]

srcs_libz11 = files([
  'compositor.cc',
  'matrices.cc',
  'render_block.cc',
  'w_compositor.cc',
  'w_compositor.h',
  'z_compositor.cc',
  'z_compositor.h',
]) + [
  wayland_server_core_hh,
  wayland_server_protocol_hh,
  z11_protocol_c,
  z11_server_protocol_h,
  z11_server_protocol_hh,
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
