project('opatIO', ['cpp', 'fortran'], version: '0.3.0a', default_options: ['cpp_std=c++23'], meson_version: '>=1.6.0')
add_project_arguments('-fvisibility=default', language: 'cpp')

subdir('build-config')
subdir('opatIO-cpp')
subdir('opatIO-fortran')

pkg = import('pkgconfig')

pkg.generate(
    name: 'opatIO',
    description: 'C++ library for OPAT',
    version: meson.project_version(),
    libraries: [libopatio],
    subdirs: ['opatIO'],
    filebase: 'opatio',
    install_dir: join_paths(get_option('libdir'), 'pkgconfig')
)

