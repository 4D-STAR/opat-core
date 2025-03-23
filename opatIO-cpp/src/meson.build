# Define the library
opatIO_sources = files(
  'private/opatIO.cpp',
)

opatIO_headers = files(
    'public/opatIO.h'
)

# Define the libopatIO library so it can be linked against by other parts of the build system
libopatIO = static_library('opatIO',
    opatIO_sources, 
    include_directories: include_directories('public'),
    cpp_args: ['-fvisibility=default'],
    dependencies: [picosha2_dep],
    install : true,
    )

opatio_dep = declare_dependency(
    include_directories: include_directories('public'),
    link_with: libopatIO,
    dependencies: [picosha2_dep]
)
# Make headers accessible
install_headers(opatIO_headers, subdir : 'opatIO/include')
