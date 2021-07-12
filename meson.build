project(
  'z11',
  'c', 'cpp',
  license : 'Apache-2.0',
  default_options : ['warning_level=3', 'werror=true', 'cpp_std=c++17', 'optimization=2'],
  meson_version : '>=0.57.0',
)

wayland_proj = subproject(
  'wayland',
  default_options : [
    'documentation=false', # avoid compilation error
    'werror=false', 'optimization=0', # back to default value
    'warning_level=2', 'buildtype=buildtype=debugoptimized' # set wayland default options
  ]
)

subdir('protocol')
subdir('wayland')
subdir('src')
subdir('clients')
