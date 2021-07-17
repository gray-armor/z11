dependencies = [
  wayland_proj.get_variable('wayland_server_dep'),
  wayland_proj.get_variable('wayland_client_dep')
]

render_block_test = executable(
  'render_block_test',
  ['main.cc'] + source_files + [ z11_client_protocol_h ],
  dependencies : dependencies,
  include_directories : src_inc,
)

test('render_block_test', render_block_test)
