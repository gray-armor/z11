project(
  'z11',
  'c', 'cpp',
  license : 'Apache-2.0',
  default_options : ['warning_level=3', 'werror=true', 'cpp_std=c++17', 'optimization=2'],
  meson_version : '>=0.57.0',
)

src_inc = include_directories('src')

subdir('protocol')
subdir('wayland')
subdir('src')
subdir('clients')
subdir('tests')
