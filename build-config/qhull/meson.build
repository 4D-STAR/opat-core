qhull_cmake_options = cmake.subproject_options()
qhull_cmake_options.add_cmake_defines({
    'CMAKE_POLICY_VERSION_MINIMUM': '3.5',
    'BUILD_SHARED_LIBS': 'ON',
    'BUILD_APPLICATIONS': 'OFF',
})
qhull_cmake_options.set_override_option('warning_level', '0')
qhull_sp = cmake.subproject('qhull', options: qhull_cmake_options)


# C++ API
qhull_cpp = qhull_sp.dependency('qhullcpp')
# C reentrant core (has all the qh_* symbols)
qhull_r   = qhull_sp.dependency('qhull_r')

# Combine them into a single array for easy reuse
qhull_dep = [ qhull_cpp, qhull_r ]