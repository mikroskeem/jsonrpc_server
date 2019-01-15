project('jsonrpc_server', 'C',
        version: '0.0.1',
        license: 'All rights reserved',
        default_options: ['c_std=c11'],
)

# Make things better and more secure
global_cflags = [
  '-Wall',
  '-Werror',
  '-Wno-error=unused-variable',
]

foreach flag : global_cflags
  add_global_arguments(flag, language : 'c')
endforeach

libjsonrpc = subproject('libjsonrpc_server')

if get_option('build_examples')
  jsonrpc_server_example = subproject('example')
endif
