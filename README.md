<p align="center">
  <img src="assets/logo/opatCoreLogo.png" width="300" alt="OPAT Core Libraries Logo">
</p>

<h1 align="center">OPAT Core Libraries</h1>

<p align="center">
  <a href="https://pypi.org/project/opatio/"><img src="https://img.shields.io/pypi/v/opatio" alt="PyPI - Version"></a>
  <a href="https://github.com/4D-STAR/opat-core/blob/main/LICENSE"><img src="https://img.shields.io/github/license/4D-STAR/opat-core" alt="GitHub License"></a>
  <a href="https://4d-star.github.io/opat-core/"><img src="https://img.shields.io/badge/Documentation-View%20Here-blue" alt="Documentation"></a>
</p>

---

This repository contains the core C++ library, python module, and file specification for the OPAT file format. 

OPAT is a structured binary file format developed by the 4D-STAR collaboration for efficient and standardized storage of set of tabular data indexed by some floating point vector (such as composition).

The general principle behind OPAT is that an arbitrary number of data cards are stored, indexed by some arbitrary-length composition vector. Each data card contains an arbitrary number of data tables indexed by some string tag.

The provided python module can be used to create and read OPAT files while the C++ module is intended to be used to interface OPAT tables with C++ code for reading (but not currently generation).

## Installation
This repository provides both C++ and python bindings. The first thing to note is that these do not depend on each other at all. If you want to generate OPAT tables and/or use them in python code you will want the python module. If you want to use OPAT tables in C++ code you will want the C++ module. 

There are more details on usage for each language in their respective folder; however, broad installation instructions are included here as well.

### Dependencies Overview

Below is a summary of the key dependencies for each part of the project.

#### Python Dependencies
These are managed by `pip` and are listed in `opatIO-py/pyproject.toml`.

| Dependency | Minimum Version | Usage                                  | Source/Authors                                                                 |
|------------|-----------------|----------------------------------------|--------------------------------------------------------------------------------|
| `numpy`    | `>= 1.21.1`     | Numerical operations, array handling   | [NumPy Developers](https://numpy.org/)                                         |
| `xxhash`   | `>= 3.5.0`      | Fast hashing algorithms (internal use) | [Yann Collet (Cyan4973)](https://github.com/Cyan4973/xxHash)                   |

#### C++ Dependencies
Most C++ dependencies are handled via Meson's wrap system (built at compile time). Boost is an exception.

| Dependency | Installation                                     | Usage                                                                 | Source/Authors                                                                                                                                                                     |
|------------|--------------------------------------------------|-----------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Boost      | System install or via bundled script (prompts user) | Numerical linear algebra (`ublas`) in `TableLattice`                  | [Boost Community](https://www.boost.org/)                                                                                                                                          |
| Qhull      | Meson wrap (built automatically)                 | N-dimensional Delaunay triangulation for `TableLattice`               | [Barber, C.B., Dobkin, D.P., and Huhdanpaa, H.T., "The Quickhull algorithm for convex hulls," ACM Trans. on Mathematical Software, 22(4):469-483, Dec 1996](http://www.qhull.org/) |
| xxHash     | Meson wrap (built automatically)                 | Fast hashing for `FloatIndexVector` lookups in `OPAT` class           | [Yann Collet (Cyan4973)](https://github.com/Cyan4973/xxHash) & [Stefan Brumme (stbrumme)](https://create.stephan-brumme.com/xxhash/)                                                                     
| PicoSHA2   | Meson wrap (built automatically)                 | SHA-256 hashing for data integrity checks (`CardCatalogEntry`)        | [Shintarou Okada (okdshin)](https://github.com/okdshin/PicoSHA2)                                                                                                                             
| cxxopts    | Meson wrap (built automatically)                 | Command-line option parsing for CLI tools (e.g., `opatHeader`)        | [Jarryd Beck (jarro2783)](https://github.com/jarro2783/cxxopts)                                                                                                                                

### Python Installation
```bash
pip install opatio
```

### C++ Installation
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
If you want to run tests you can use meson's build in test command. Note that due to small numerical differences between computers and compilers, some tests may fail. This is expected and can be ignored. Specifically, `TableLattice` tests rely on the ordering / adjacency of Delaunay triangulation which can vary between systems.
```bash
meson test -C build
```


To install headers, libraries, and the command line utilities
```bash
meson install -C build
```

---

# Usage

## Python Usage
See the `opatIO-py` directory for detailed installation and usage instructions. The Python module allows for both creating and reading OPAT files. A comprehensive API manual can also be found in `opatIO-py/docs/build/latex/opatio.pdf`.

The general workflow involves creating an `OPAT` object, adding tables to it, and then saving it.

### Creating and Writing an OPAT File
```python
from opatio import OPAT

# Initialize an OPAT object
opacityFile = OPAT()
opacityFile.set_comment("This is a sample opacity file")
opacityFile.set_source("OPLIB") # Optional: set a source identifier

# Assume you have your data:
# X, Z: components of the index vector (e.g., Hydrogen and Metal mass fractions)
# logR: 1D array of log(R) values (e.g., density variable)
# logT: 1D array of log(T) values (e.g., temperature variable)
# logKappa: 2D array of log(opacity) values, with shape (len(logR), len(logT))
X, Z = 0.7, 0.02
logR = [1.0, 2.0, 3.0] 
logT = [4.0, 5.0, 6.0, 7.0]
logKappa = [[1.1, 1.2, 1.3, 1.4], [2.1, 2.2, 2.3, 2.4], [3.1, 3.2, 3.3, 3.4]]


# Add a table to a new DataCard, indexed by (X, Z)
# The tag "data" identifies this specific table within the DataCard
card = opacityFile.add_table(index_vector=(X, Z), 
                             tag="data", 
                             row_values=logR, 
                             col_values=logT, 
                             table_data=logKappa, 
                             rowName="logR", 
                             columnName="logT")

# You can add another table to the same DataCard
# For example, interpolation coefficients
# xOrder, yOrder, coeff = ... (your coefficient data)
# opacityFile.add_table(index_vector=(X,Z), tag="interp_coeffs", 
#                       row_values=x_coeffs_indices, col_values=y_coeffs_indices, table_data=coeffs,
#                       card=card) # Pass the existing card object

# Save the OPAT file
opacityFile.save("opacity.opat")

# Optionally, save an ASCII representation (for debugging or human-readable inspection)
opacityFile.save_as_ascii("opacity_ascii.txt")

print("OPAT file created and saved.")
```

### Reading an OPAT File
```python
from opatio import read_opat

# Load an existing OPAT file
opacityFile = read_opat("opacity.opat")

# Print the header information
print(opacityFile.header)

# Access a specific DataCard by its index vector
# Note: Index vectors are tuples. For floating point comparisons, 
# direct dictionary-like access might require exact matches.
# It's often better to iterate or use methods that handle floating point precision if needed.
# For this example, we assume the index (0.7, 0.02) exists.
target_index = (0.7, 0.02)
if target_index in opacityFile.cards:
    data_card = opacityFile.cards[target_index]
    
    # Access a specific table within the DataCard by its tag
    if "data" in data_card.tables:
        table = data_card.tables["data"]
        print(f"\nTable 'data' for index {target_index}:")
        print(f"Row Names (logR): {table.row_values}")
        print(f"Column Names (logT): {table.col_values}")
        print(f"Table Data (logKappa):\n{table.table_data}")
    else:
        print(f"Table 'data' not found in card with index {target_index}")
else:
    print(f"DataCard with index {target_index} not found.")

# Iterate through all cards and their tables
for index_vec, card in opacityFile.cards.items():
    print(f"\nDataCard at index: {index_vec}")
    for tag, table in card.tables.items():
        print(f"  Table tag: {tag}, Shape: ({table.num_rows}, {table.num_cols})")
```

### Converting OPAL Type I to OPAT
A utility is provided to convert common OPAL Type I opacity files to the OPAT format.
```python
from opatio.convert import OPALI_2_OPAT

# Assuming an OPAL Type I file named "GS98hz" is in the current directory
OPALI_2_OPAT("GS98hz", "gs98hz.opat")
print("Converted OPAL Type I file to gs98hz.opat")
```

## C++ Library/Header Usage
The C++ library is primarily designed for reading and interpolating data from OPAT files. Below are some common usage examples. For a more detailed API manual, refer to the Doxygen documentation generated in `opatIO-cpp/docs/html/index.html` (or the PDF version in `opatIO-cpp/docs/latex/refman.pdf`).

### Basic OPAT File Operations

#### Reading an OPAT File
```c++
#include "opatIO.h" // Main header for OPAT file operations
#include <string>
#include <iostream>

int main() {
    std::string filename = "example.opat";
    try {
        opat::OPAT opat_file = opat::readOPAT(filename);
        std::cout << "Successfully read OPAT file: " << filename << std::endl;
        opat_file.header.print(); // Print the header information
    } catch (const std::exception& e) {
        std::cerr << "Error reading OPAT file: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

#### Accessing a Table by Index and Tag

```c++
#include "opatIO.h"
#include "indexVector.h" // For FloatIndexVector
#include <string>
#include <vector>
#include <iostream>

int main() {
    std::string filename = "example.opat";
    try {
        opat::OPAT opat_file = opat::readOPAT(filename);
        // Define the index vector for the DataCard you want to access
        FloatIndexVector target_index({0.35, 0.004}); // Example index
        // Define the tag for the table within the DataCard
        std::string table_tag = "data"; // Example tag

        const opat::DataCard& data_card = opat_file[target_index];
        const opat::OPATTable& table = data_card[table_tag];
        
        std::cout << "Table '" << table_tag << "' at index " << target_index << ":" << std::endl;
        table.print(); // Print the table data
    } catch (const std::exception& e) {
        std::cerr << "Error accessing table: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

#### Slicing a Table
```c++
#include "opatIO.h"
#include "indexVector.h"
#include <string>
#include <vector>
#include <iostream>

int main() {
    std::string filename = "example.opat";
    try {
        opat::OPAT opat_file = opat::readOPAT(filename);
        FloatIndexVector target_index({0.35, 0.004});
        std::string table_tag = "data";

        // Define slices for rows and columns
        opat::Slice row_slice(0, 6);    // Rows 0 through 5
        opat::Slice col_slice(25, 36);  // Columns 25 through 35

        const opat::OPATTable& original_table = opat_file[target_index][table_tag];
        opat::OPATTable sliced_table = original_table.slice(row_slice, col_slice);
        
        std::cout << "Sliced table '" << table_tag << "' at index " << target_index << ":" << std::endl;
        sliced_table.print(); // Print the sliced table
    } catch (const std::exception& e) {
        std::cerr << "Error slicing table: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

### Using `TableLattice` for Interpolation
The `TableLattice` class allows for N-dimensional linear interpolation of data within an OPAT file.

```c++
#include "opatIO.h"
#include "indexVector.h"
#include "tableLattice.h" // For TableLattice
#include <string>
#include <vector>
#include <iostream>

int main() {
    std::string opat_filename = "your_data.opat"; // Replace with your OPAT file
    try {
        opat::OPAT opat_data = opat::readOPAT(opat_filename);

        // Initialize TableLattice with the loaded OPAT data
        opat::lattice::TableLattice lattice(opat_data);
        std::cout << "TableLattice initialized." << std::endl;

        // Define the index vector for which you want to interpolate data
        // Ensure its dimension matches opat_data.header.numIndex
        FloatIndexVector query_point({0.54, 0.07}); // Example for a 2D OPAT

        // Get the interpolated DataCard
        opat::DataCard interpolated_card = lattice.get(query_point);
        std::cout << "Interpolated DataCard retrieved for " << query_point << std::endl;

        // Access tables from the interpolated_card as needed
        // For example, if your OPAT files contain a table tagged "density":
        // if (!interpolated_card.getKeys().empty()) {
        //     const opat::OPATTable& density_table = interpolated_card[interpolated_card.getKeys()[0]];
        //     density_table.print();
        // }

    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

## Command Line Utility Usage
After running `meson install -C build`, three command-line utilities will be available in your system's path (you might need to resource your shell, e.g., `source ~/.bashrc` or `source ~/.zshrc`, or open a new terminal). These utilities are:

1.  **`opatHeader`**: Prints out the main header information of an OPAT file.
    ```bash
    opatHeader --file path/to/your/file.opat
    ```

2.  **`opatInspect`**: Prints the main header and the card catalog (index of all data cards) of an OPAT file.
    ```bash
    opatInspect --file path/to/your/file.opat
    ```

3.  **`opatVerify`**: Verifies if the provided file is a valid OPAT file according to the specification. It checks the magic number and performs other integrity checks.
    ```bash
    opatVerify --file path/to/your/file.opat
    ```

Note the `--file` (or `-f`) flag is required before providing the path to the OPAT file for all tools.

---

## Fortran Usage
A Fortran wrapper around the C++ `opatIO` module is provided. This interface allows Fortran applications to read data from OPAT files. Note that this is primarily for reading and may not receive the same level of feature development as the C++ and Python interfaces.

The Fortran module and object files are typically built into `build/opatIO-fortran/libopatio_f.a` (and associated `.mod` files in `build/opatIO-fortran/libopatio_f.a.p`). You will need to link against this library and potentially the C++ `opatIO` library as well.

Refer to `opatIO-fortran/readme.md` and `opatIO-cpp/docs/static/fortran.md` for more details.

### Example: Reading an OPAT File in Fortran

The core workflow involves:
1. Loading the OPAT file using `load_opat_file`.
2. Retrieving a specific table using `get_opat_table`.
3. Checking the returned table structure for errors.
4. Accessing table data.
5. Freeing resources with `free_opat_file`.

```fortran
program opat_fortran_example
    use opat_interface_module
    use, intrinsic :: iso_c_binding
    implicit none

    character(len=256)      :: filename
    real(c_double), target  :: index_vec(2) ! Assuming 2D index vector
    character(len=32)       :: tag          ! Ensure tag length matches definition in module
    type(opat_table_f_type) :: my_table
    integer                 :: load_status
    integer                 :: i, j

    ! --- Configuration ---
    filename = 'gs98hz.opat'  ! Ensure this file exists where the program is run
                              ! A copy can be found in opatIO-cpp/examples
    index_vec(1) = 0.9_c_double   ! Example X value
    index_vec(2) = 0.08_c_double  ! Example Z value
    tag = 'data'              ! Tag of the table to retrieve

    ! --- Load OPAT File ---
    print *, "Attempting to load OPAT file: ", trim(filename)
    load_status = load_opat_file(trim(filename))
    if (load_status /= 0) then
        print *, "ERROR: Failed to load OPAT file. Status: ", load_status
        stop 1
    end if
    print *, "OPAT file loaded successfully."

    ! --- Get Table ---
    print *, "Attempting to retrieve table with tag '", trim(tag), "' for index [", index_vec(1), ",", index_vec(2), "]"
    call get_opat_table(index_vec, trim(tag), my_table)

    ! --- Check for Errors and Process Table ---
    if (my_table%error_code /= 0) then
        print *, "ERROR: Failed to retrieve table. Error code: ", my_table%error_code
        if (my_table%error_code == 1) print *, " (Likely cause: Index vector not found)"
        if (my_table%error_code == 2) print *, " (Likely cause: Table tag not found for given index)"
    else
        print *, "Table retrieved successfully!"
        print *, "Table dimensions: ", my_table%num_rows, "rows, ", my_table%num_cols, "cols"

        ! Check if pointers are associated (important!)
        if (.not. c_associated(my_table%row_values)) then
            print *, "ERROR: Row values pointer not associated."
        else if (.not. c_associated(my_table%col_values)) then
            print *, "ERROR: Column values pointer not associated."
        else if (.not. c_associated(my_table%data)) then
            print *, "ERROR: Table data pointer not associated."
        else
            ! Access and print some data (example)
            print *, "First row value: ", my_table%row_values(1)
            print *, "First col value: ", my_table%col_values(1)
            print *, "Data at (1,1): ", my_table%data(1,1)

            ! Example: Print a small part of the table
            ! print *, "Sample data:"
            ! do i = 1, min(my_table%num_rows, 3)
            !    write(*,'(3(F8.3,X))') (my_table%data(i,j), j = 1, min(my_table%num_cols,3))
            ! end do
        end if
    end if

    ! --- Free OPAT File Resources ---
    print *, "Freeing OPAT file resources."
    call free_opat_file()
    print *, "Program finished."

end program opat_fortran_example
```
**Important Notes for Fortran Usage:**
- Ensure the OPAT file (e.g., `gs98hz.opat`) is accessible in the directory where the Fortran executable is run.
- Always check `my_table%error_code` after calling `get_opat_table`.
- Crucially, call `free_opat_file()` when you are done with the OPAT file to prevent memory leaks.
- The dimensions of `index_vec` must match the `numIndex` defined in the OPAT file's header.
- The length of the `tag` character variable should be sufficient for the tags used in your OPAT files.
