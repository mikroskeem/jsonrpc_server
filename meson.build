project('jsonrpc_server', 'C',
        version: '0.0.1',
        license: 'All rights reserved',
        default_options: ['c_std=c11'],
)

# Make things better and more secure
project_cflags = [
  '-Wall',
  '-Werror',
  '-Wno-error=unused-variable',
  '-fstack-protector-strong',
]

foreach flag : project_cflags
  add_project_arguments(flag, language : 'c')
endforeach

cc = meson.get_compiler('c')

server_inc = include_directories('src')
server_sources = [
  'src/generic_errors.c',
  'src/jsonrpc.c',
  'src/main.c',
]

server_dependencies = [
  dependency('jansson', version: '>=2.12'),
  dependency('zlib', version: '>=1.2.8'),
]

executable('jsonrpc_server',
           server_sources,
           include_directories: server_inc,
           dependencies: server_dependencies
)
