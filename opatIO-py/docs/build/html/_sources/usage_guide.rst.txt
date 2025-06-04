===========
Usage Guide
===========

This guide explains how to use the ``opatio`` library to work with OPAT (Open Parameterized Array Table) files.

Core Concepts
-------------

* **OPAT Object:** The main object (``opatio.OPAT``) represents an entire OPAT file in memory. It holds the header, a catalog of data cards, and the data cards themselves.
* **Index Vector:** A tuple of floating-point numbers that parameterizes a specific data set (e.g., ``(temperature, density)``). The number of elements in this tuple must match the ``numIndex`` value set in the OPAT header.
* **Data Card:** A container within the OPAT file associated with a specific Index Vector. It holds one or more tables relevant to that specific parameter combination.
* **Table:** A 2D or 3D array of data within a Data Card, identified by a string ``tag``. Each table has associated row and column axis values (e.g., ``logR``, ``logT``).

Creating a New OPAT File
------------------------

Follow these steps to create an OPAT file from scratch:

1.  **Import necessary libraries:**

    .. code-block:: python

       import opatio
       import numpy as np

2.  **Create an OPAT instance:**

    .. code-block:: python

       opat = opatio.OPAT()

3.  **Set Header Information (Optional but Recommended):**
    Define metadata for your file. Crucially, set the number of dimensions for your index vectors using ``set_numIndex``.

    .. code-block:: python

       opat.set_comment("Opacity data for stellar modeling")
       opat.set_source("Generated from Model X calculations")
       opat.set_numIndex(2) # Expecting 2D index vectors, e.g., (logT, logRho)

4.  **Prepare Your Data:**
    Define the index vector, table tag, axis values, and the main data array for each table you want to add.

    .. code-block:: python

       # Example data for a specific (logT, logRho) combination
       index_vec_1 = (4.5, -3.0) # Example: logT = 4.5, logRho = -3.0
       tag_1 = "rosseland_mean"

       # Axis values for the table
       logR_values = np.array([-8.0, -7.5, -7.0]) # Example: log(R) where R = rho / T_6^3
       logT_values = np.array([3.0, 5.0, 7.0])

       # The 2D data array (shape must match len(rowValues) x len(columnValues))
       opacity_data_1 = np.array([
           [0.1, 0.5, 1.0],
           [0.2, 0.8, 1.5],
           [0.4, 1.2, 2.0]
       ])

5.  **Add the Table to the OPAT object:**
    Use the ``add_table`` method. This method handles creating or updating the appropriate ``DataCard`` associated with the ``indexVector``.

    .. code-block:: python

       opat.add_table(
           indexVector=index_vec_1,
           tag=tag_1,
           columnValues=logR_values,     # Data corresponding to columns
           rowValues=logT_values,        # Data corresponding to rows
           data=opacity_data_1,
           columnName="logR",           # Optional descriptive name for column axis
           rowName="logT"     # Optional descriptive name for row axis
       )

       # Add more tables for the same or different index vectors as needed
       # index_vec_2 = (5.0, -2.5)
       # opat.add_table(indexVector=index_vec_2, tag="planck_mean", ...)

6.  **Save the OPAT file:**
    Write the in-memory OPAT object to a binary file.

    .. code-block:: python

       output_filename = "stellar_opacities.opat"
       try:
           opat.save(output_filename)
           print(f"OPAT file saved to {output_filename}")
       except Exception as e:
           print(f"Error saving file: {e}")

    You can also save a human-readable (but non-standard) ASCII representation for debugging:

    .. code-block:: python

       opat.save_as_ascii("stellar_opacities_debug.dat")


Loading an Existing OPAT File
-----------------------------

To read data from an OPAT file:

1.  **Import the library:**

    .. code-block:: python

       import opatio

2.  **Use `read_opat`:**

    .. code-block:: python

       input_filename = "stellar_opacities.opat"
       loaded_opat = opatio.read_opat(input_filename)
       print(f"Successfully loaded {input_filename}")

3.  **Access Header Information:**
    Metadata is stored in the ``header`` attribute.

    .. code-block:: python

       print(f"Comment: {loaded_opat.header.comment}")
       print(f"Source: {loaded_opat.header.sourceInfo}")
       print(f"Number of Index Dimensions: {loaded_opat.header.numIndex}")
       print(f"Number of Data Cards: {loaded_opat.header.numCards}")

4.  **Access Data Cards and Tables:**
    Data cards are stored in the ``cards`` dictionary, keyed by ``FloatVectorIndex`` objects. Tables within a card are accessed like dictionary items using their string tag.

    .. code-block:: python

       # Define the index vector you want to retrieve data for
       target_index_vec = (4.5, -3.0)
       target_tag = "rosseland_mean"

       target_card = loaded_opat[target_index_vec]

       target_table = target_card[target_tag]

       # Access the table's components
       print(f"Table Tag: {target_tag}")
       print(f"Column Axis ({target_card.index[target_tag].columnName}): {target_table.columnValues}")
       print(f"Row Axis ({target_card.index[target_tag].rowName}): {target_table.rowValues}")
       print(f"Data Array (shape {target_table.data.shape}):\n{target_table.data}")


Modifying an OPAT File
----------------------

You can load an OPAT file, modify it (add/remove cards/tables), and save the changes.

1.  **Load the file** as shown above.
2.  **Add a new table or card** using ``loaded_opat.add_table(...)`` or ``loaded_opat.add_card(...)``.
3.  **Remove an existing card** using ``loaded_opat.pop_card(index_vector)``. Note this removes the entire card associated with that index vector.
4.  **Re-save the file** using ``loaded_opat.save(input_filename)`` (overwriting) or ``loaded_opat.save("modified_opacities.opat")`` (saving to a new file).

.. code-block:: python

   # Example: Removing a card (assuming loaded_opat exists)
   index_to_remove_tuple = (5.0, -2.5)
   index_to_remove_key = FloatVectorIndex(
       vector=index_to_remove_tuple,
       hashPrecision=loaded_opat.header.hashPrecision
   )

   try:
       removed_card = loaded_opat.pop_card(index_to_remove_key)
       print(f"Removed card for index {index_to_remove_tuple}")
       # Now save the modified opat object
       # loaded_opat.save("modified_file.opat")
   except KeyError:
       print(f"Card for index {index_to_remove_tuple} not found, nothing removed.")


Converting OPAL Type I Files
----------------------------

The library includes a utility to convert OPAL Type I opacity files to the OPAT format.

.. code-block:: python

   from opatio.convert import OPALI_2_OPAT

   opal_input_file = "path/to/your/opal_file.GN93"
   opat_output_file = "converted_opal.opat"

   OPALI_2_OPAT(opal_input_file, opat_output_file)
   print(f"Converted {opal_input_file} to {opat_output_file}")

   # Optionally save an ASCII version for inspection
   OPALI_2_OPAT(opal_input_file, opat_output_file, saveAsASCII=True)
   print(f"Also saved ASCII debug file.")


This provides a basic overview. Refer to the API Reference section for detailed information on specific classes and methods.

Storing 3D Data
----------------
To store 3D data, you can use the `add_table` method with a 3D numpy array. The method will automatically handle the additional dimension.
For example, if you have a 2D array of temperature and density and for each point you want to store a vector of interpolation coefficients
you can do the following:

.. code-block:: python

   # Example data for a specific (logT, logR) combination
   tag_1 = "rosseland_mean"
   # Axis values for the table
   logR_values = np.array([-8.0, -7.5, -7.0]) # Example: log(R) where R = rho / T_6^3
   logT_values = np.array([3.0, 5.0, 7.0])

   # The 3D data array (shape must match len(rowValues) x len(columnValues) x num_coefficients)
   opacity_data_1 = np.random.rand(3, 3, 4)  # Random data for demonstration

   opat.add_table(
       tag=tag_1,
       columnValues=logR_values,
       rowValues=logT_values,
       data=opacity_data_1,
       columnName="logR",
       rowName="logT"
   )


The shape will automatically be inferred from the data you provide. The `add_table` method will handle the additional dimension and store it correctly in the OPAT file.

Using TableLattice for Interpolation
-----------------------------

The `TableLattice` class provides a powerful mechanism for interpolating data stored in an OPAT object using Delaunay triangulation. This is particularly useful for estimating data for index vectors that are not explicitly present in the OPAT file.

Example: Basic Interpolation
===========================

1. **Import the necessary classes:**

   .. code-block:: python

      from opatio.base.opat import OPAT
      from opatio.lattice.tableLattice import TableLattice
      from opatio.index.floatvectorindex import FloatVectorIndex

2. **Load or create an OPAT object:**

   .. code-block:: python

      opat = read_opat("path/to/your/opat_file.opat")
      # Add data to the OPAT object (see previous sections for details)

3. **Create a `TableLattice` instance:**

   .. code-block:: python

      lattice = TableLattice(opat)

4. **Query for interpolated data:**

   .. code-block:: python

      query_vector = FloatVectorIndex((4.75, -3.25))  # Example query vector
      try:
          interpolated_card = lattice.get(query_vector)
          print("Interpolation successful!")
      except ValueError as e:
          print(f"Interpolation failed: {e}")

5. **Access the interpolated table:**

   .. code-block:: python

      tag = "rosseland_mean"
      interpolated_table = interpolated_card[tag]
      print(f"Interpolated data for tag '{tag}':")
      print(interpolated_table.data)

Advanced Example: Interpolating Multiple Tables
================================================

You can interpolate multiple tables simultaneously if the data cards have consistent tags.

.. code-block:: python

   query_vector = FloatVectorIndex((4.75, -3.25))
   interpolated_card = lattice.get(query_vector)

   for tag in interpolated_card.keys():
       interpolated_table = interpolated_card[tag]
       print(f"Interpolated data for tag '{tag}':")
       print(interpolated_table.data)

Potential Pitfalls and Gotchas
============================

1. **Query Point Outside Triangulation:**
   If the query vector lies outside the convex hull of the index vectors, interpolation will fail with a `ValueError`. Ensure that your query vector is within the range of the data.

   .. code-block:: python

      query_vector = FloatVectorIndex((10.0, -5.0))  # Outside the convex hull
      try:
          interpolated_card = lattice.get(query_vector)
      except ValueError:
          print("Query point is outside the triangulation.")

2. **Inconsistent Tags Across Data Cards:**
   The interpolation assumes that all data cards in the OPAT object have the same set of tags. If tags differ between cards, interpolation will fail. Verify consistency before using `TableLattice`.

   .. code-block:: python

      # Check tags consistency
      tags = [set(card.keys()) for card in opat.cards.values()]
      if not all(tags[0] == t for t in tags):
          print("Tags are inconsistent across data cards.")

3. **Collinear Points:**
   If the index vectors are collinear, Delaunay triangulation cannot be constructed. Ensure that your index vectors span a valid space.

   .. code-block:: python

      try:
          lattice = TableLattice(opat)
      except ValueError as e:
          print(f"Failed to build triangulation: {e}")

4. **Precision Issues:**
   Floating-point precision can affect the triangulation and interpolation. Use consistent precision for index vectors and query vectors.

   .. code-block:: python

      query_vector = FloatVectorIndex((4.7500001, -3.2500001))  # Slightly off due to precision
      interpolated_card = lattice.get(query_vector)

Summary
========

The `TableLattice` class is a robust tool for interpolating data stored in OPAT files. By understanding its functionality and addressing potential pitfalls, you can effectively use it to estimate data for arbitrary index vectors.
