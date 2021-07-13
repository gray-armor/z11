source_files = [
  'simple_box.c',
  wayland_server_core_hh,
  wayland_server_protocol_hh,
  z11_protocol_c,
  z11_client_protocol_h,
]

dependencies = [
  wayland_proj.get_variable('wayland_client_dep')
]

executable(
  'z11-simple-box',
  source_files,
  install : true,
  dependencies : dependencies,
)
