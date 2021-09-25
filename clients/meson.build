simple_box_source_files = [
  'z_window.cc',
  'simple_box.cc',
  z11_protocol_c,
  z11_client_protocol_h,
  z11_input_protocol_c,
  z11_input_client_protocol_h,
  z11_opengl_protocol_c,
  z11_opengl_client_protocol_h,
]

png_viewer_source_files = [
  'z_window.cc',
  'png_viewer.cc',
  'png_loader.cc',
  z11_protocol_c,
  z11_client_protocol_h,
  z11_input_protocol_c,
  z11_input_client_protocol_h,
  z11_opengl_protocol_c,
  z11_opengl_client_protocol_h,
]

dependencies = [
  dependency('wayland-client'),
  dependency('libpng'),
]

executable(
  'z11-simple-box',
  simple_box_source_files,
  install : true,
  dependencies : dependencies,
  include_directories : public_inc,
)

executable(
  'z11-png-viewer',
  png_viewer_source_files,
  install : true,
  dependencies : dependencies,
  include_directories : public_inc,
)
