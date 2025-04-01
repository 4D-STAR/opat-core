# Test files for opatIO
gtest_dep = dependency('gtest', main: true, required : true)
gtest_main = dependency('gtest_main', required: true)
gtest_nomain_dep = dependency('gtest', main: false, required : true)

test_sources = [
    'opatIOTest.cpp',
]



foreach test_file : test_sources
  exe_name = test_file.split('.')[0]
  message('Building test: ' + exe_name)

  # Create an executable target for each test
  test_exe = executable(
      exe_name,
      test_file,
      dependencies: [gtest_dep, picosha2_dep, gtest_main, opatio_dep, xxhash_dep],
      install_rpath: '@loader_path/../../src'  # Ensure runtime library path resolves correctly
  )

  # Add the executable as a test
  test(
    exe_name,
    test_exe,
    env: ['MESON_SOURCE_ROOT=' + meson.project_source_root(), 'MESON_BUILD_ROOT=' + meson.project_build_root()])
endforeach
