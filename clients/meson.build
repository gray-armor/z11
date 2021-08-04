simle_box_source_files = [
  'helper.c',
  'helper.h',
  'simple_box.c',
  z11_protocol_c,
  z11_client_protocol_h,
]

png_viewer_source_files = [
  'helper.c',
  'helper.h',
  'png_viewer.c',
  z11_protocol_c,
  z11_client_protocol_h,
]

dependencies = [
  dependency('wayland-client'),
  dependency('libpng'),
  meson.get_compiler('c').find_library('m', require : false),
]

executable(
  'z11-simple-box',
  simle_box_source_files,
  install : true,
  dependencies : dependencies,
)

executable(
  'z11-png-viewer',
  png_viewer_source_files,
  install : true,
  dependencies : dependencies,
)
