dep_compositor = [
  libzazen_public_dep,
  dependency('sdl2'),
  dependency('openvr'),
  dependency('glew'),
]

src_compositor = [
  'main.cc',
  'eye.cc',
  'eye.h',
  'matrices.cc',
  'matrices.h',
  'renderer.cc',
  'renderer.h',
  'sdl.cc',
  'sdl.h',
  'hmd.cc',
  'hmd.h',
  'shader.cc',
  'shader.h',
  'vectors.h',
  'z_server.cc',
  'z_server.h',
]

executable(
  'zazen',
  src_compositor,
  install : true,
  dependencies : dep_compositor,
)
