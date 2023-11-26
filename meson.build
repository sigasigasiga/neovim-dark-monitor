project('neovim-dark-monitor', 'cpp', default_options : [ 'cpp_std=c++20' ])

boost_dep = dependency('boost', modules: ['program_options'])

# TODO: add them as submodules?
spdlog_dep = dependency('spdlog')
ranges_dep = dependency('range-v3')
msgpack_dep = dependency('msgpack-cxx')

sources = [
  'src/main.cpp',

  'src/monitor/service/inventory.cpp',
  'src/monitor/service/singleton.cpp',

  'src/monitor/singleton/client.cpp',
  'src/monitor/singleton/client_handler.cpp',
  'src/monitor/singleton/server.cpp',

  'src/monitor/util/inventory.cpp',
  'src/monitor/util/msgpack_socket_read.cpp',
]

executable(
  meson.project_name(),
  sources,
  dependencies: [boost_dep, msgpack_dep, spdlog_dep, ranges_dep],
  include_directories: ['src/'],
  cpp_pch: 'src/shared_pch.hpp'
)
