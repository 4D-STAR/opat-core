# OpatIO Core Libraries
This repository contains the core C++ library, python module, and file specification for the OPAT file format. 

OPAT is a structured binary file format developed by the 4D-STAR collaboration for efficient and standardized storage of set of tabular data indexed by some floating point vector (such as composition).

The general principle behind OPAT is that an arbitrary number of data cards are stored, indexed by some arbitrary-length composition vector. Each data card containes an arbitrary number of data tables indexed by some string tag.

The provided python module can be used to create and read OPAT files while the C++ module is intended to be used to interface OPAT tables with C++ code for reading (but not currently generation).

## Installation
This repository provides both C++ and python bindings. The first thing to note is that these do not depend on each other at all. If you want to generate OPAT tables and/or use them in python code you will want the python module. If you want to use OPAT tables in C++ code you will want the C++ module. 

There are more details on usage for each language in their respective folder; however, broad installation instructions are included here as well.

### Python Installation
```bash
pip install opatio
```

### C++ Installation
You will need `meson`, `cmake`, and `ninja` installed pre-installed. These can be installed with pip
```bash
pip install meson>=1.6.0
pip install cmake
pip install ninja
```
Then you can build and install opat-core
```bash
git clone https://github.com/4D-STAR/opat-core
cd opat-core
meson setup build --buildtype=release
meson compile -C build
```
If you want to run tests
```bash
meson test -C build
```
To install headers, libraries, and the command line utilities
```
meson install -C build
```
# Usage

## Python Usage
See opatIO-py for installation and usage instructions. A more detailed API manual can be found in `opatIO-py/docs/build/latex/opatio.pdf`.

## C++ Library/Header Usage
See OpatIO-cpp for cpp building and installation instructions. A more detailed API manual can be found in `opatIO-cpp/docs/latex/refman.pdf`.

## Command Line Utility Usage
If you run `meson install` there will be three command line utilities avalible in your path (you may have to resource your shell either by closing and reopening the terminal or by typing `source ~/.zshrc` if on zsh or `source ~/.bashrc` if on bash. These utilities are `opatHeader`, `opatInspect`, and `opatVerify`. They: print out the header, print the header and the table index, print if the provided file is a valid OPAT file respectivley. Usgae of these tools looks like

```bash
opatHeader -f path/to/file.opat
```

Note the `-f`, that is needed before providing the path to the file. Usage for all tools looks the same.
