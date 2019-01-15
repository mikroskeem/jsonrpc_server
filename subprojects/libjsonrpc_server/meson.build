project('libjsonrpc_server', 'C',
        version: '0.0.1',
        license: 'MIT'
)

pkg = import('pkgconfig')

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

libjsonrpc_server = library('jsonrpc_server',
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

pkg.generate(libjsonrpc_server)
install_headers(headers, subdir: 'jsonrpc_server')
