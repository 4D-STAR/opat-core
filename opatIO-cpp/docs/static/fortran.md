# Fortran Interface

Using the `opatIO-cpp` module we provide a rudimentary fotran interface. Note that this interface does not
support all the same feature as the C++ interface and will not recive the same degree of support and development
that the C++ and python interfaces will. The Fortran interface is primarily intended for use in legacy code bases.

## Example: Reading an OPAT File in Fortran

Below is an example program demonstrating how to use the Fortran interface to load an OPAT file, retrieve a table, and access its data.

### Key Steps:
1. Load the OPAT file using `load_opat_file`.
2. Retrieve a specific table using `get_opat_table`, providing an index vector and a tag.
3. Check the returned table structure for errors.
4. Access the table's data via associated pointers (`row_values`, `col_values`, `data`).
5. Clean up resources using `free_opat_file`.

### Example Code:
```fortran
program opat_read_example
    use opat_interface_module
    use, intrinsic :: iso_c_binding
    implicit none

    CHARACTER(LEN=256) :: filename
    REAL(C_DOUBLE), TARGET :: index_vec(2)
    CHARACTER(LEN=4) :: tag
    TYPE(opat_table_f_type) :: my_table
    INTEGER :: load_status

    filename = 'gs98hz.opat'
    index_vec = [0.9, 0.08]
    tag = "data"

    load_status = load_opat_file(TRIM(filename))
    IF (load_status /= 0) THEN
        PRINT *, "ERROR: Failed to load OPAT file."
        STOP 1
    END IF

    CALL get_opat_table(index_vec, tag, my_table)
    IF (my_table%error_code /= 0) THEN
        PRINT *, "ERROR retrieving table."
    ELSE
        PRINT *, "Table retrieved successfully!"
        ! Access table data here...
    END IF

    CALL free_opat_file()
end program
```

### Notes:
- Always check the `error_code` in the returned table structure to ensure the operation was successful.
- Ensure pointers (`row_values`, `col_values`, `data`) are associated before accessing them.
- Call `free_opat_file` to release resources when done.
