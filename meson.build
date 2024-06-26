project('neovim-dark-monitor', 'cpp', default_options : [ 'cpp_std=c++20' ])

boost_dep = dependency('boost', modules: ['program_options'])
spdlog_dep = dependency('spdlog')
ranges_dep = dependency('range-v3')
msgpack_dep = dependency('msgpack-cxx')
tl_expected_dep = dependency('tl-expected')
darknotify_dep = subproject('libdarknotify').get_variable('darknotify_dep')

sources = [
  'src/main.cpp',

  'src/monitor/application/application.cpp',
  'src/monitor/application/mode/monitor.cpp',
  'src/monitor/application/mode/singleton_client.cpp',

  'src/monitor/rpc/client.cpp',
  'src/monitor/rpc/dispatcher.cpp',
  'src/monitor/rpc/error.cpp',
  'src/monitor/rpc/message.cpp',

  'src/monitor/service/inventory.cpp',
  'src/monitor/service/monitor.cpp',
  'src/monitor/service/neovim.cpp',
  'src/monitor/service/request_handler.cpp',
  'src/monitor/service/singleton.cpp',

  'src/monitor/singleton/client.cpp',
  'src/monitor/singleton/client_handler.cpp',
  'src/monitor/singleton/server.cpp',

  'src/monitor/util/inventory.cpp',

  'src/monitor/util/asio/endpoint.cpp',

  'src/monitor/util/os/user.cpp',
]

executable(
  'neovim-dark-monitor.exe',
  sources,
  dependencies: [boost_dep, msgpack_dep, spdlog_dep, ranges_dep, darknotify_dep, tl_expected_dep],
  include_directories: ['src/'],
  cpp_pch: 'src/shared_pch.hpp'
)
