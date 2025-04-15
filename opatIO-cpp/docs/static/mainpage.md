# OPAT I/O Library Documentation

## Overview

The `opatIO-cpp` library is a high-performance C++ library designed for reading, writing, and manipulating OPAT (Open Parameterized Array Table) files. These files are used to store structured numeric, in a compact and efficient format, indexed by some arbitrary length floating point vector. The library provides both programmatic and command-line interfaces, making it suitable for integration into larger software systems or for standalone data inspection and validation tasks.

Key features of `opatIO-cpp` include:
- **C++ API**: A robust API for accessing and manipulating OPAT files programmatically.
- **Data Slicing**: Support for slicing tables by rows and columns for efficient data extraction.
- **Raw Data Access**: Direct access to raw data for advanced use cases.
- **CLI Tools**: Command-line utilities for inspecting, validating, and analyzing OPAT files.
- **Cross-Platform**: Compatible with major operating systems, including Linux, macOS, and Windows.
- **Performance**: Optimized for speed and low memory overhead, making it suitable for large datasets.

The library is part of the broader OPAT ecosystem and is designed to work seamlessly with other tools and libraries in the OPAT suite.

### Note

The `opatIO-cpp` library is intended only for reading OPAT files. Writing OPAT files is not supported at this time. The library is designed to be fast and efficient, but it does not include all the features of the OPAT file format. For more advanced use cases, consider using the Python version of the library.

---

## Installation
You will need `meson`, `cmake`, and `ninja` installed pre-installed. These can be installed with pip
```bash
pip install "meson>=1.6.0"
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
---

## Usage

### 1. Programmatic (API) Usage

The OPAT I/O library provides a C++ API for interacting with OPAT files. Below are some common use cases:

#### Reading an OPAT File
```cpp
#include "opatIO.h"

int main() {
    std::string filename = "example.opat";
    opat::OPAT file = opat::readOPAT(filename);
    file.header.print(); // Print the header information
    return 0;
}
```

#### Accessing a Table by Index and Tag
```cpp
#include "opatIO.h"

int main() {
    opat::OPAT file = opat::readOPAT("example.opat");
    FloatIndexVector index({0.35, 0.004});
    const auto& table = file[index]["data"];
    table.print(); // Print the table data
    return 0;
}
```

#### Slicing a Table
```cpp
#include "opatIO.h"

int main() {
    opat::OPAT file = opat::readOPAT("example.opat");
    FloatIndexVector index({0.35, 0.004});
    opat::Slice rowSlice(0, 6);
    opat::Slice colSlice(25, 36);
    auto slicedTable = file[index]["data"].slice(rowSlice, colSlice);
    slicedTable.print(); // Print the sliced table
    return 0;
}
```

#### Retrieving Raw Data
```cpp
#include "opatIO.h"

int main() {
    opat::OPAT file = opat::readOPAT("example.opat");
    FloatIndexVector index({0.35, 0.004});
    const double* rawData = file[index]["data"].getRawData();
    // Use rawData as needed
    return 0;
}
```

---

### 2. Command-Line Interface (CLI) Usage

The OPAT I/O library includes several CLI tools for inspecting and validating OPAT files. Below are the available tools and their usage:

#### 2.1 `opatHeader`
**Description:** Displays the header information of an OPAT file.

**Usage:**
```bash
opatHeader --file <path_to_opat_file>
```

**Example:**
```bash
opatHeader --file example.opat
```

#### 2.2 `opatInspect`
**Description:** Displays the header and card catalog information of an OPAT file.

**Usage:**
```bash
opatInspect --file <path_to_opat_file>
```

**Example:**
```bash
opatInspect --file example.opat
```

#### 2.3 `opatVerify`
**Description:** Verifies if a file is a valid OPAT file.

**Usage:**
```bash
opatVerify --file <path_to_opat_file>
```

**Example:**
```bash
opatVerify --file example.opat
```

---

## Additional Notes

- For more detailed examples and API documentation, refer to the header files (`opatIO.h`, `indexVector.h`) and the source code.
- Ensure that the OPAT file format adheres to the expected structure to avoid runtime errors.

`