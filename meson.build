project('neovim-dark-monitor', 'cpp', default_options : [ 'cpp_std=c++20' ])

# FIXME: comment this line out. it is needed only to fix clangd errors
add_project_arguments('-includesrc/shared_pch.hpp', language : 'cpp')

boost_dep = dependency('boost', modules: ['program_options'])

# TODO: add them as submodules?
spdlog_dep = dependency('spdlog')
ranges_dep = dependency('range-v3')
msgpack_dep = dependency('msgpack-cxx')

darknotify_proj = subproject('libdarknotify')
darknotify_dep = darknotify_proj.get_variable('darknotify_dep')

sources = [
  'src/main.cpp',

  'src/monitor/application/application.cpp',
  'src/monitor/application/mode/monitor.cpp',
  'src/monitor/application/mode/singleton_client.cpp',

  'src/monitor/neovim/client.cpp',

  'src/monitor/service/inventory.cpp',
  'src/monitor/service/monitor.cpp',
  'src/monitor/service/neovim.cpp',
  'src/monitor/service/singleton.cpp',

  'src/monitor/singleton/client.cpp',
  'src/monitor/singleton/client_handler.cpp',
  'src/monitor/singleton/server.cpp',

  'src/monitor/util/error.cpp',
  'src/monitor/util/inventory.cpp',

  'src/monitor/util/asio/endpoint.cpp',

  'src/monitor/util/os/user.cpp',
]

executable(
  meson.project_name(),
  sources,
  dependencies: [boost_dep, msgpack_dep, spdlog_dep, ranges_dep, darknotify_dep],
  include_directories: ['src/'],
  cpp_pch: 'src/shared_pch.hpp'
)
