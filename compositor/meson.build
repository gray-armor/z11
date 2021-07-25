dep_compositor = [
  libz11_public_dep,
  dependency('sdl2'),
  dependency('glew'),
]

src_compositor = [
  'main.cc',
  'eye.cc',
  'eye.h',
  'renderer.cc',
  'renderer.h',
  'sdl.cc',
  'sdl.h',
  'shader.cc',
  'shader.h',
]

executable(
  meson.project_name(),
  src_compositor,
  install : true,
  dependencies : dep_compositor,
)
