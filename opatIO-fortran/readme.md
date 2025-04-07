# Fortran OPAT-IO module
A fortran wrapper around the C++ opatIO module. Note that this is a test project and may not recive the same support at opatio (python) or opatIO-cpp (C++).

This library will be built when you build the main project.

There are module and object files that will end up in build/opatIO-fortran/libopatio_f.a.p
    - These will likely need to be included and linked against your fortran source if you wish to use opat files in fortran code

## General Usage
There is an example file in opatIO-fortran/tests/sandbox which demonstrates the usage of this library. Note that when you run this test executable you will need the file gs98hz.opat in your current working directory. There is a copy of this file in opatIO-cpp/examples.

The key ideas is that the user:
    1. Opens an opat file
    2. Interacts with it
    3. *manually* closses it (this is important to prevent memory leaks)

## Notes
This fortran module relies on the C++ opatIO backend, therefore, that object file will liekely also need to be linked in to your executable.