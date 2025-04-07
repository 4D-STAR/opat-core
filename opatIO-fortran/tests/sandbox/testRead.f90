program opat_read_example
    use opat_interface_module
    use, intrinsic :: iso_c_binding
    implicit none

    ! --- Variables ---
    CHARACTER(LEN=256) :: filename         ! Path to the OPAT file
    REAL(C_DOUBLE), TARGET :: index_vec(2) ! Example: 3D index vector. Must have TARGET attribute.
    CHARACTER(LEN=4) :: tag              ! 4-character tag for the desired table
    TYPE(opat_table_f_type) :: my_table  ! Variable to hold the returned table data/pointers
    INTEGER :: load_status      ! Status code from loading the file
    INTEGER :: i, j             ! Loop variables

    filename = 'gs98hz.opat' ! <<< SET YOUR OPAT FILENAME HERE
    index_vec = [0.9, 0.08]
    tag = "data"

    PRINT *, "Attempting to load OPAT file: ", TRIM(filename)
    load_status = load_opat_file(TRIM(filename))

    ! Check if loading was successful
    IF (load_status /= 0) THEN
        PRINT *, "ERROR: Failed to load OPAT file. Check filename and C++ backend. Stopping."
        STOP 1 ! Stop execution if file loading fails
    END IF
    PRINT *, "OPAT file loaded successfully."
    PRINT *, "------------------------------------"

    ! 2. Request a specific table using the index vector and tag
    PRINT *, "Attempting to retrieve table..."
    PRINT *, "  Index Vector:", index_vec
    PRINT *, "  Table Tag   :", TRIM(tag)

    ! Call the wrapper subroutine. 'my_table' will be populated.
    CALL get_opat_table(index_vec, tag, my_table)

    PRINT *, "------------------------------------"

    ! 3. Check the error code returned in the Fortran table structure
    IF (my_table%error_code /= 0) THEN
        PRINT *, "ERROR retrieving table:"
        IF (ALLOCATED(my_table%error_message)) THEN
             PRINT *, "  Message: ", TRIM(my_table%error_message)
        ELSE
             PRINT *, "  (No specific error message returned from C++)"
        END IF
        PRINT *, "  Error Code:", my_table%error_code
    ELSE
        ! 4. Success! Access the table data via the pointers
        PRINT *, "Table retrieved successfully!"
        PRINT *, "  Dimensions (Rows x Columns): ", my_table%num_rows, " x ", my_table%num_cols

        ! Always check if pointers are associated before using them
        IF (ASSOCIATED(my_table%row_values)) THEN
            PRINT *, "  Row Values (first few):"
            WRITE(*,'(5(F10.4,X))') (my_table%row_values(i), i = 1, MIN(5, my_table%num_rows))
        ELSE
             PRINT *, "  Row values pointer not associated."
        END IF

        IF (ASSOCIATED(my_table%col_values)) THEN
            PRINT *, "  Column Values (first few):"
            WRITE(*,'(5(F10.4,X))') (my_table%col_values(i), i = 1, MIN(5, my_table%num_cols))
        ELSE
             PRINT *, "  Column values pointer not associated."
        END IF

        IF (ASSOCIATED(my_table%data)) THEN
            PRINT *, "  Table Data (Top-Left Corner):"
            DO i = 1, MIN(5, my_table%num_rows)
                WRITE(*,'(5(F10.4,X))') (my_table%data(i, j), j = 1, MIN(5, my_table%num_cols))
            END DO
            ! Now you can work with the data array: my_table%data(row, column)
        ELSE
            PRINT *, "  Table data pointer not associated."
        END IF
    END IF


    ! 5. IMPORTANT: Clean up C++ resources when completely finished
    ! This invalidates any pointers previously obtained (row_values, col_values, data)
    PRINT *, "------------------------------------"
    PRINT *, "Calling C++ cleanup function..."
    CALL free_opat_file()
    PRINT *, "Cleanup called."

    ! Pointers in my_table are now dangling (pointing to freed memory)
    ! Good practice to nullify them, although the program is ending anyway
    NULLIFY(my_table%row_values, my_table%col_values, my_table%data)

    PRINT *, "------------------------------------"
    PRINT *, "Fortran program finished."


end program