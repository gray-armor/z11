source_files = [
  'main.cc',
  'compositor.cc',
  'compositor.h',
  'render_block.cc',
  'render_block.h',
  'w_compositor.cc',
  'w_compositor.h',
  'z_compositor.cc',
  'z_compositor.h',
  wayland_server_core_hh,
  wayland_server_protocol_hh,
  z11_protocol_c,
  z11_server_protocol_h,
  z11_server_protocol_hh,
]

dependencies = [
  dependency('wayland-server'),
]

executable(
  meson.project_name(),
  source_files,
  install : true,
  dependencies : dependencies,
)
