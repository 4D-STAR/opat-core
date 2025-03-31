=========================
C++ Utilities Usage Guide
=========================

This guide explains how to build, install, and use the command-line utilities
provided by the C++ implementation within the `opat-core` project.

Dependencies
------------

To build the C++ utilities, you will need the following build system tools:

* **Meson:** The primary build system used for the project.
* **CMake:** Required by Meson for certain tasks, particularly dependency detection.

Additionally, the build process requires the following libraries:

* **gtest (Google Test):** Used for running unit tests.
* **xxHash:** Used for hashing functionalities.

**Internet Connection Requirement:** If `gtest` and `xxHash` are not already installed on your system in a way that Meson/CMake can find them, the Meson build process is configured to automatically download them using its WrapDB feature. Therefore, an **internet connection may be required** during the `meson setup` step if these dependencies need to be fetched.

Building from Source
--------------------

Follow these steps to compile the C++ utilities:

1.  **Clone the Repository:**
    If you haven't already, clone the `opat-core` repository:

    .. code-block:: bash

       git clone https://github.com/4D-STAR/opat-core.git
       cd opat-core

2.  **Configure the Build with Meson:**
    Use Meson to set up the build directory. Running this from the repository root creates a `build` subdirectory. The `--buildtype=release` flag optimizes the build.

    .. code-block:: bash

       meson setup build --buildtype=release

3.  **Compile the Code:**
    Use Meson to compile the source code within the configured build directory.

    .. code-block:: bash

       meson compile -C build

Upon successful compilation, the executables (`opatHeader`, `opatVerify`, `opatInspect`) will be located within the `build` directory.

Installation
------------

To install the compiled utilities to a system-wide location (e.g., `/usr/local/bin`, depending on your system configuration and permissions), run the following command from the repository root:

.. code-block:: bash

   meson install -C build

You might need administrator privileges (e.g., using `sudo`) depending on the installation prefix configured by Meson.

Headers
-------

In order to use opatIO from within another C++ program you will need to link against the opatIO library and include the opatIO.h header file.

  .. code-block:: cpp

    #include "opatIO.h"

    int main() {
      OpatIO opat("gs98hz.opat");
      double kappa = opat.get([0.01, 0.001])["data"]["table"][0][0];
    }
  
The complication here with all of the indexing is that we access the first tag in the data card at 0.01, 0.001 (this first tag is called data).
Then we get the table from that (as opposed to the row and column values, such as logR and logT), then we get the 0,0 element from the table.


Command-Line Utilities
----------------------

The build process generates three command-line utilities for interacting with OPAT files:

opatHeader
~~~~~~~~~~

* **Purpose:** Prints the main header information from a specified OPAT file.
* **Usage:**

  .. code-block:: bash

     opatHeader -f <path_to_opat_file>

  * Replace `<path_to_opat_file>` with the actual path to your `.opat` file.

opatVerify
~~~~~~~~~~

* **Purpose:** Validates the structure and integrity of an OPAT file to ensure it conforms to the format specification. It checks headers, offsets, and potentially checksums.
* **Usage:**

  .. code-block:: bash

     opatVerify -f <path_to_opat_file>

  * Replace `<path_to_opat_file>` with the actual path to your `.opat` file.

opatInspect
~~~~~~~~~~~

* **Purpose:** Prints both the main header information and the card catalog entries from a specified OPAT file. This gives a more detailed overview than `opatHeader`.
* **Usage:**

  .. code-block:: bash

     opatInspect -f <path_to_opat_file>

  * Replace `<path_to_opat_file>` with the actual path to your `.opat` file.

Example
-------

To view the header of a file named `my_data.opat` located in the current directory, you would run:

.. code-block:: bash

   opatHeader -f my_data.opat






