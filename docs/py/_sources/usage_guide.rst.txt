===========
Usage Guide
===========

This guide explains how to use the ``opatio`` library to work with OPAT (Open Parameterized Array Table) files.

Core Concepts
-------------

* **OPAT Object:** The main object (``opatio.OPAT``) represents an entire OPAT file in memory. It holds the header, a catalog of data cards, and the data cards themselves.
* **Index Vector:** A tuple of floating-point numbers that parameterizes a specific data set (e.g., ``(temperature, density)``). The number of elements in this tuple must match the ``numIndex`` value set in the OPAT header.
* **Data Card:** A container within the OPAT file associated with a specific Index Vector. It holds one or more tables relevant to that specific parameter combination.
* **Table:** A 2D array of data within a Data Card, identified by a string ``tag``. Each table has associated row and column axis values (e.g., ``logR``, ``logT``).

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
       frequency_values = np.array([1e14, 1e15, 1e16]) # Example: Frequency in Hz

       # The 2D data array (shape must match len(rowValues) x len(columnValues))
       opacity_data_1 = np.array([
           [0.1, 0.5, 1.0], # Opacity at freq=1e14 for logR = -8.0, -7.5, -7.0
           [0.2, 0.8, 1.5], # Opacity at freq=1e15 for logR = -8.0, -7.5, -7.0
           [0.4, 1.2, 2.0]  # Opacity at freq=1e16 for logR = -8.0, -7.5, -7.0
       ])

5.  **Add the Table to the OPAT object:**
    Use the ``add_table`` method. This method handles creating or updating the appropriate ``DataCard`` associated with the ``indexVector``.

    .. code-block:: python

       opat.add_table(
           indexVector=index_vec_1,
           tag=tag_1,
           columnValues=logR_values,     # Data corresponding to columns
           rowValues=frequency_values,   # Data corresponding to rows
           data=opacity_data_1,
           columnName="logR",           # Optional descriptive name for column axis
           rowName="Frequency (Hz)"     # Optional descriptive name for row axis
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
       from opatio.index import FloatVectorIndex # Needed for accessing cards by index

2.  **Use `read_opat`:**

    .. code-block:: python

       input_filename = "stellar_opacities.opat"
       try:
           loaded_opat = opatio.read_opat(input_filename)
           print(f"Successfully loaded {input_filename}")
       except FileNotFoundError:
           print(f"Error: File not found at {input_filename}")
           exit()
       except Exception as e:
           print(f"Error loading OPAT file: {e}")
           exit()

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
       target_index_vec_tuple = (4.5, -3.0)
       target_tag = "rosseland_mean"

       # Create the FloatVectorIndex key (using precision from the loaded header)
       target_index_key = FloatVectorIndex(
           vector=target_index_vec_tuple,
           hashPrecision=loaded_opat.header.hashPrecision
       )

       # Check if the card exists and access it
       if target_index_key in loaded_opat.cards:
           target_card = loaded_opat.cards[target_index_key]
           print(f"\nFound data card for index {target_index_vec_tuple}")

           # Access the specific table within the card by its tag
           if target_tag in target_card.tables:
               target_table = target_card[target_tag] # Access via __getitem__

               # Access the table's components
               print(f"Table Tag: {target_tag}")
               print(f"Column Axis ({target_card.index[target_tag].columnName}): {target_table.columnValues}")
               print(f"Row Axis ({target_card.index[target_tag].rowName}): {target_table.rowValues}")
               print(f"Data Array (shape {target_table.data.shape}):\n{target_table.data}")
           else:
               print(f"Table with tag '{target_tag}' not found in this card.")
       else:
           print(f"Data card for index {target_index_vec_tuple} not found in the file.")


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

   try:
       OPALI_2_OPAT(opal_input_file, opat_output_file)
       print(f"Converted {opal_input_file} to {opat_output_file}")

       # Optionally save an ASCII version for inspection
       OPALI_2_OPAT(opal_input_file, opat_output_file, saveAsASCII=True)
       print(f"Also saved ASCII debug file.")

   except FileNotFoundError:
       print(f"Error: Input OPAL file not found: {opal_input_file}")
   except Exception as e:
       print(f"Error during conversion: {e}")

This provides a basic overview. Refer to the API Reference section for detailed information on specific classes and methods.
