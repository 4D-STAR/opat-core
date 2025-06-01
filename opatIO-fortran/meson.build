opatio_f_lib = static_library('opatio_f',
  sources: [
    'src/opat_c_types_module.f90',
    'src/opatio.f90',
  ],
  include_directories: ['src', '../opatIO-cpp/src/public'],
  dependencies: [picosha2_dep, xxhash_dep],
  link_with: [libopatio],
)

opatio_f_dep = declare_dependency(
  include_directories: ['src'],
  link_with: [opatio_f_lib],
)

subdir('tests')