scanner = dependency('wayland-scanner')
scanner_path = scanner.get_variable(pkgconfig : 'wayland_scanner')
z11_xml = files('z11.xml')

z11_client_protocol_h = custom_target(
  'z11-client-protocol',
  output : 'z11-client-protocol.h',
  input : z11_xml,
  command : [scanner_path, 'client-header', '@INPUT@', '@OUTPUT@'],
)

z11_server_protocol_h = custom_target(
  'z11-server-protocol',
  output : 'z11-server-protocol.h',
  input : z11_xml,
  command : [scanner_path, 'server-header', '@INPUT@', '@OUTPUT@'],
)

z11_protocol_c = custom_target(
  'z11-protocol',
  output : 'z11-protocol.c',
  input : z11_xml,
  command : [scanner_path, 'public-code', '@INPUT@', '@OUTPUT@'],
)

install_data(z11_xml)
