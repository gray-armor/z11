client_source_files = [
  'simple_box.c',
  z11_protocol_c,
  z11_client_protocol_h,
]

dependencies = [
  dependency('wayland-client')
]

executable(
  'z11-simple-box',
  client_source_files,
  install : true,
  dependencies : dependencies,
)
