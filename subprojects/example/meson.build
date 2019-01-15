project('jsonrpc_server_example', 'C',
        version: '0.0.1',
        license: 'MIT',
)

libjsonrpc_server = subproject('libjsonrpc_server')
libjsonrpc_server_dep = libjsonrpc_server.get_variable('libjsonrpc_server_dep')
libjsonrpc_server_dependencies = libjsonrpc_server.get_variable('dependencies')

sources = [
  'src/main.c'
]

dependencies = [
  dependency('zlib', version: '>=1.2.3'),
  dependency('openssl', version: '>=1.1.1a'),
  libjsonrpc_server_dep
]
dependencies += libjsonrpc_server_dependencies

executable('jsonrpc_server_example',
           sources,
           dependencies: dependencies
)
