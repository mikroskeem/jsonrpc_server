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
  '-fvisibility=hidden',
]

foreach flag : project_cflags
  add_project_arguments(flag, language : 'c')
endforeach

cc = meson.get_compiler('c')

project_inc = include_directories('src')

library_sources = [
  'src/generic_errors.c',
  'src/jsonrpc.c',
]

server_sources = [
  'src/main.c',
]

library_dependencies = [
  dependency('jansson', version: '>=2.12'),
]

server_dependencies = [
  dependency('zlib', version: '>=1.2.3'),
  dependency('openssl', version: '>=1.1.1a'),
]
server_dependencies += library_dependencies

lib = shared_library('jsonrpc_server',
        library_sources,
        include_directories: project_inc,
        dependencies: library_dependencies
)

executable('jsonrpc_server',
           server_sources,
           include_directories: project_inc,
           dependencies: server_dependencies,
           link_with: [ lib ]
)
