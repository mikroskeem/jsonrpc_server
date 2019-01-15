project('libjsonrpc_server', 'C')

project_cflags = [
  '-fvisibility=hidden',
]

foreach flag : project_cflags
  add_project_arguments(flag, language : 'c')
endforeach

project_inc = include_directories('src')

sources = [
  'src/generic_errors.c',
  'src/jsonrpc.c',
]

# Note: Public API only
headers = [
  'src/jsonrpc.h'
]

dependencies = [
  dependency('jansson', version: '>=2.12'),
]

libjsonrpc_server = shared_library('jsonrpc_server',
        sources,
        include_directories: project_inc,
        dependencies: dependencies,
        soversion : '0',
        install: true
)

libjsonrpc_server_dep = declare_dependency(
  include_directories: project_inc,
  link_with: [ libjsonrpc_server ]
)

install_headers(headers, subdir: 'jsonrpc_server')
