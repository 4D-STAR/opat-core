# OpatIO-cpp
C++ module for interacting with OPAT files. 

## Dependecies
In order to build this project you will need `meson >= 1.6.0` and `ninja` installed. This can be most easily done with `pip`

```bash
pip install meson
pip install ninja
```

## Building
Building is done with meson 

```bash
git clone https://github.com/4D-STAR/opat-core
cd opat-core
meson setup build --buildtype=release
meson compile -C build
```

## Running Tests
```bash
meson test -C build
```

## Installing
```bash
meson install -C build
```

## Usage
when installing three executables will be installed. These should end up in your path but if they do not they are also in build/tools. These are

- opatHeader : Display the header of an OPAT file
- opatVerify : Verify if a file is a valid OPAT file
- opatInspect : Display the header and table index of an OPAT file

all of these tools have the same usage pattern

```bash
opatHeader -f <path/to/file>
```
