project(
  'z11',
  'c', 'cpp',
  license : 'Apache-2.0',
  default_options : ['warning_level=3', 'werror=true', 'cpp_std=c++17', 'optimization=2'],
  meson_version : '>=0.57.0',
)

libzazen_major = 0
libzazen_revision = 0

public_inc = include_directories('include')

pkgconfig = import('pkgconfig')

wayland_server_dep = dependency('wayland-server')
wayland_client_dep = dependency('wayland-client')

cglm_dep = dependency('cglm', fallback : ['cglm', 'cglm_dep'])

subdir('protocol')
subdir('libzazen')
subdir('zazen')
subdir('clients')
subdir('tests')
