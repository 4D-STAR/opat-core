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

## Dependencies

The `opatIO-cpp` project relies on several external libraries to provide its functionality. Understanding these dependencies can be helpful for development and troubleshooting.

-   **Boost**:
    -   **Usage**: Primarily used in the `TableLattice` component for numerical linear algebra operations, specifically `boost::numeric::ublas::matrix` and `boost::numeric::ublas::vector` for solving systems of equations related to barycentric coordinate calculations.
    -   **Installation**: Boost is the only dependency that typically needs to be installed as a system library. However, the project includes a helper script (`build-config/boost/install.sh`) that can download and build Boost locally if it's not detected on your system. The Meson build system will prompt you before attempting to run this script.

-   **Qhull**:
    -   **Usage**: Utilized by the `TableLattice` class to perform N-dimensional Delaunay triangulation of the OPAT index points, which is a core part of the interpolation mechanism.
    -   **Installation**: Qhull is managed via Meson's wrap system and is built automatically during the project's compilation if not found. No separate system installation is typically required.

-   **xxHash**:
    -   **Usage**: A fast hashing algorithm used to find the hash of a FloatIndexVector for fast lookups in the `OPAT` class. This is crucial for efficiently accessing data in OPAT files.
    -   **Installation**: Managed via Meson's wrap system and built automatically.

-   **PicoSHA2**:
    -   **Usage**: Used for generating SHA-256 hashes to validate data integrity.
    -   **Installation**: Managed via Meson's wrap system and built automatically.

-   **cxxopts**:
    -   **Usage**: A lightweight C++ library for parsing command-line options. This is used by the command-line interface (CLI) tools provided with `opatIO-cpp` (e.g., `opatHeader`, `opatInspect`, `opatVerify`) to handle user-provided arguments.
    -   **Installation**: Managed via Meson's wrap system and built automatically.

In summary, for most users, only ensuring Boost is available (or allowing the build script to install it) is necessary. The other dependencies are handled seamlessly by the Meson build system.

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

## Using `TableLattice` for Interpolation

The `opatIO-cpp` library includes a powerful feature for interpolating data within OPAT files: the `TableLattice` class. This is particularly useful when you need to estimate data values at index points that are not explicitly defined in the OPAT file.

### Theoretical Background

The `TableLattice` works by performing N-dimensional Delaunay triangulation on the index vectors found in the provided OPAT file. Each unique index vector in the OPAT data becomes a vertex in this triangulation.

When you request data for a specific query point (an index vector):
1.  **Simplex Search**: The `TableLattice` first identifies the N-simplex (e.g., a triangle in 2D, a tetrahedron in 3D) within the Delaunay triangulation that encloses your query point. This is done using a "walk" algorithm through the triangulation.
2.  **Barycentric Coordinates**: Once the containing simplex is found, the barycentric coordinates of the query point with respect to the vertices of that simplex are calculated. Barycentric coordinates represent the query point as a weighted average of the simplex's vertices.
3.  **Interpolation**: The data associated with each vertex of the containing simplex (i.e., the `DataCard`s from the original OPAT file) are then interpolated using these barycentric weights. Currently, only **linear interpolation** is supported. This means each table within the `DataCard`s is interpolated element-wise.

The result is a new `DataCard` containing tables where each value is an interpolated estimate corresponding to your query point.

### Initialization

To use `TableLattice`, you first need an `opat::OPAT` object, which you can get by reading an OPAT file. Then, you construct a `TableLattice` object, passing the `opat::OPAT` object to its constructor.

```cpp
#include "opatIO.h"       // For opat::readOPAT, opat::OPAT
#include "tableLattice.h" // For opat::lattice::TableLattice
#include <string>

int main() {
    std::string opat_filename = "your_data.opat";
    opat::OPAT opat_data = opat::readOPAT(opat_filename);

    // Initialize TableLattice with the loaded OPAT data
    opat::lattice::TableLattice lattice(opat_data);

    // By default, Linear interpolation is used.
    // You can also specify it explicitly:
    // opat::lattice::TableLattice lattice_linear(opat_data, opat::lattice::InterpolationType::Linear);

    return 0;
}
```
During construction, the `TableLattice` will:
1. Extract all unique index vectors from the `opat_data`.
2. Build the Delaunay triangulation of these index vectors. This step can throw a `std::runtime_error` if, for example, there are insufficient or degenerate points for triangulation.

### Usage: Getting Interpolated Data

Once the `TableLattice` is initialized, you can get interpolated data using its `get()` method. This method takes a `FloatIndexVector` (representing the point at which you want to interpolate) and returns an `opat::DataCard`.

```cpp
#include "opatIO.h"
#include "indexVector.h"  // For FloatIndexVector
#include "tableLattice.h"
#include <iostream>

int main() {
    std::string opat_filename = "your_data.opat";
    opat::OPAT opat_data = opat::readOPAT(opat_filename);
    opat::lattice::TableLattice lattice(opat_data);

    // Define the index vector for which you want to interpolate data
    // Ensure its dimension matches opat_data.header.numIndex
    FloatIndexVector query_point({0.54, 0.07}); // Example for a 2D OPAT

    // Get the interpolated DataCard
    // This can throw exceptions (see Gotchas section)
    opat::DataCard interpolated_card = lattice.get(query_point);

    // Now you can access tables from the interpolated_card as usual
    // For example, if your OPAT files contain a table tagged "density":
    // const opat::OPATTable& density_table = interpolated_card["density"];
    // density_table.print();

    std::cout << "Successfully retrieved interpolated data for " << query_point << std::endl;

    return 0;
}

```

### Gotchas and Considerations

-   **Interpolation Type**: Currently, only `InterpolationType::Linear` is implemented and supported. Attempting to set or use other types will result in a `std::runtime_error`.
-   **Out of Bounds/Convex Hull**:
    -   The `get()` method will throw a `std::out_of_range` exception if the query `indexVector` is outside the rectangular bounds defined by the minimum and maximum values of the original index vectors in each dimension.
    -   More subtly, even if a point is within these rectangular bounds, it might be outside the *convex hull* of the original data points. If the `findContainingSimplex` method cannot find a simplex that encloses the query point (i.e., the point is outside the convex hull), it will also typically result in a `std::out_of_range` or `std::runtime_error` (e.g., if the walk algorithm fails to locate the point).
    -   **Resolution**: Ensure your query points are within the domain covered by the input OPAT file's index vectors.
-   **Dimensionality**: The query `indexVector` must have the same number of dimensions as the index vectors in the OPAT file (`opat_data.header.numIndex`). Mismatched dimensions will lead to a `std::invalid_argument` exception.
-   **Degenerate Data**: If the index points in the OPAT file are degenerate (e.g., all points are collinear in 2D, or coplanar in 3D, preventing a valid N-dimensional triangulation), the `TableLattice` constructor might fail (throwing `std::runtime_error` from Qhull).
    -   **Resolution**: Ensure your OPAT file contains a good distribution of index points suitable for N-dimensional triangulation.
-   **Performance**:
    -   The construction of `TableLattice` (building the Delaunay triangulation) can be computationally intensive for a very large number of unique index vectors or high dimensionality. However, this is a one-time cost.
    -   The `get()` operation involves a walk through the triangulation and then barycentric calculations. The `TableLattice` caches the last found simplex, which can speed up queries for spatially coherent points.
-   **Error Handling**: Methods like `get()`, constructors, and `setInterpolationType()` can throw various exceptions (`std::out_of_range`, `std::invalid_argument`, `std::runtime_error`). It's good practice to wrap calls to these methods in `try-catch` blocks in production code if you anticipate potentially problematic inputs.
-   **Dumping Triangulation**: For debugging or visualization, you can dump the triangulation structure:
    ```cpp
    lattice.dumpTriangulationToAscii("points_output.txt", "simplices_output.txt");
    ```

The `TableLattice` provides a robust way to perform N-dimensional linear interpolation on OPAT data, extending the utility of your datasets.

---


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
