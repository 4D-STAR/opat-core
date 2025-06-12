# Define the library
opatio_sources = files(
  'private/opatIO.cpp',
  'private/indexVector.cpp',
  'private/tableLattice.cpp',
  'private/fextern.cpp'
)

opatio_headers = files(
  'public/opatIO.h',
  'public/indexVector.h',
  'public/tableLattice.h'
)

dependencies = [
    picosha2_dep,
    xxhash_dep,
    qhull_dep,
    boost_dep
]

# Define the libopatIO library so it can be linked against by other parts of the build system
libopatio = shared_library('opatio',
    opatio_sources, 
    include_directories: include_directories('public'),
    cpp_args: ['-fvisibility=default'],
    dependencies: dependencies,
    install : true,
    )

opatio_dep = declare_dependency(
    include_directories: include_directories('public'),
    link_with: libopatio,
    dependencies: dependencies
)
# Make headers accessible
install_headers(opatio_headers, subdir : 'opatIO')
