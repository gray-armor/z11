scanner = wayland_proj.get_variable('wayland_scanner')

z11_client_protocol_h = custom_target(
  'z11-client-protocol',
  output : 'z11-client-protocol.h',
  input : 'z11.xml',
  command : [scanner, 'client-header', '@INPUT@', '@OUTPUT@'],
)

z11_server_protocol_h = custom_target(
  'z11-server-protocol',
  output : 'z11-server-protocol.h',
  input : 'z11.xml',
  command : [scanner, 'server-header', '@INPUT@', '@OUTPUT@'],
)

z11_protocol_c = custom_target(
  'z11-protocol',
  output : 'z11-protocol.c',
  input : 'z11.xml',
  command : [scanner, 'public-code', '@INPUT@', '@OUTPUT@'],
)
