source_files = [
  'simple_box.c',
  wayland_server_core_hh,
  wayland_server_protocol_hh,
  z11_protocol_c,
  z11_client_protocol_h,
]

dependencies = [
  dependency('wayland-client'),
]

executable(
  'z11-simple-box',
  source_files,
  install : true,
  dependencies : dependencies,
)
