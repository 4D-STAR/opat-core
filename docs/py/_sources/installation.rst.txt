.. _Install:
============
Installation
============

This page provides instructions on how to install the ``opatio`` library.

Prerequisites
-------------

Before installing ``opatio``, ensure you have the following prerequisites installed:

* **Python:** Version 3.x is recommended.
* **pip:** The Python package installer. It usually comes with Python. You can upgrade it using ``pip install --upgrade pip``.

Dependencies
------------

``opatio`` relies on the following Python packages:

* **numpy:** For numerical operations and array handling.
* **xxhash:** For fast hashing, used internally for indexing.

These dependencies are typically installed automatically when you install ``opatio`` using ``pip`` (see below).

* **sphinx:** Required only if you intend to build the documentation from source. It is not needed just to use the ``opatio`` library.

You can manually install dependencies if needed:

.. code-block:: bash

   pip install numpy xxhash

Installing from PyPI
--------------------

The recommended way to install ``opatio`` is from the Python Package Index (PyPI) using ``pip``. This will install the latest stable release and automatically handle the required dependencies (``numpy``, ``xxhash``).

Open your terminal or command prompt and run:

.. code-block:: bash

   pip install opatio

Installing from Source (GitHub)
-------------------------------

If you want the latest development version or want to contribute to the project, you can install ``opatio`` directly from the source code on GitHub. This method will also install the required dependencies.

1.  **Clone the repository:**
    First, clone the `opat-core` repository from GitHub:

    .. code-block:: bash

       git clone https://github.com/4D-STAR/opat-core.git

2.  **Navigate to the Python package directory:**
    Change into the specific directory containing the Python package setup:

    .. code-block:: bash

       cd opat-core/opatIO_py

3.  **Install the package:**
    Use ``pip`` to install the package in editable mode (recommended for development) or standard mode.

    * **Editable mode:**

        .. code-block:: bash

           pip install -e .

    * **Standard mode:**

        .. code-block:: bash

           pip install .

This will install the ``opatio`` package and its runtime dependencies from your local clone.

Verify Installation
-------------------

After installation, you can verify it by trying to import the package in a Python interpreter:

.. code-block:: python

   import opatio
   print("opatio imported successfully!")

If the import succeeds without errors, the installation was successful.
