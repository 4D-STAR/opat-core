! opat_interface_module.f90
MODULE opat_interface_module
    USE, INTRINSIC :: ISO_C_BINDING
    USE opat_c_types_module           ! <<< ADD THIS LINE
    IMPLICIT NONE

    PRIVATE

    TYPE :: opat_table_f_type
        REAL(C_DOUBLE), POINTER :: row_values(:) => NULL()
        REAL(C_DOUBLE), POINTER :: col_values(:) => NULL()
        REAL(C_DOUBLE), POINTER :: data(:,:) => NULL()
        INTEGER :: num_rows = 0
        INTEGER :: num_cols = 0
        INTEGER :: error_code = 1
        CHARACTER(:), ALLOCATABLE :: error_message
    END TYPE opat_table_f_type

    ! --- Interface Block (Changed to SUBROUTINE) ---
    INTERFACE
        ! load_opat_file_c (remains function)
        INTEGER(C_INT) FUNCTION load_opat_file_c(filename) BIND(C, name='load_opat_file_c')
            USE, INTRINSIC :: ISO_C_BINDING
            IMPLICIT NONE
            CHARACTER(KIND=C_CHAR), INTENT(IN) :: filename(*)
        END FUNCTION load_opat_file_c

        ! free_opat_file_c (remains subroutine)
         SUBROUTINE free_opat_file_c() BIND(C, name='free_opat_file_c')
             USE, INTRINSIC :: ISO_C_BINDING
             IMPLICIT NONE
         END SUBROUTINE free_opat_file_c

        ! **** MODIFIED INTERFACE ****
        ! Now a SUBROUTINE taking the result type as an argument
        SUBROUTINE get_opat_table_c(index_vector_ptr, index_vector_size, table_tag, result_out) &
            BIND(C, name='get_opat_table_c')
            USE, INTRINSIC :: ISO_C_BINDING
            use opat_c_types_module
            IMPLICIT NONE
            TYPE(C_PTR), VALUE, INTENT(IN) :: index_vector_ptr
            INTEGER(C_INT), VALUE, INTENT(IN) :: index_vector_size
            CHARACTER(KIND=C_CHAR), INTENT(IN) :: table_tag(*)
            TYPE(opat_table_c_type), INTENT(OUT) :: result_out ! Pass struct as output argument
        END SUBROUTINE get_opat_table_c
    END INTERFACE

    ! --- Public Interface (remains the same) ---
    PUBLIC :: load_opat_file, free_opat_file, get_opat_table, opat_table_f_type


CONTAINS ! --- Module Procedures ---

    ! load_opat_file (remains the same)
    FUNCTION load_opat_file(filename) RESULT(error_code)
        CHARACTER(LEN=*), INTENT(IN) :: filename
        INTEGER :: error_code
        INTEGER(C_INT) :: c_error_code
        c_error_code = load_opat_file_c(TRIM(filename) // C_NULL_CHAR)
        error_code = INT(c_error_code)
        IF (error_code /= 0) THEN
             PRINT *, "Fortran: Error loading OPAT file: ", filename, " Code: ", error_code
        ELSE
             PRINT *, "Fortran: OPAT file loaded successfully: ", filename
        END IF
    END FUNCTION load_opat_file

     ! free_opat_file (remains the same)
    SUBROUTINE free_opat_file()
         CALL free_opat_file_c()
         PRINT *, "Fortran: Called C++ function to free OPAT file."
    END SUBROUTINE free_opat_file

    ! **** MODIFIED Fortran WRAPPER ****
    ! Now calls the SUBROUTINE interface
    SUBROUTINE get_opat_table(index_vector, tag, fortran_table)
        use, intrinsic :: ISO_C_BINDING
        use opat_c_types_module
        REAL(C_DOUBLE), TARGET, INTENT(IN) :: index_vector(:)
        CHARACTER(LEN=*), INTENT(IN) :: tag
        TYPE(opat_table_f_type), INTENT(OUT) :: fortran_table ! Changed to INTENT(OUT)

        TYPE(opat_table_c_type) :: c_result      ! Local variable to receive C struct data
        TYPE(C_PTR)             :: c_index_ptr   ! Pointer to Fortran index data
        INTEGER(C_INT)          :: c_index_size  ! Size of index vector for C
        ! Local copies of C result components
        INTEGER(C_INT)          :: local_error_code
        TYPE(C_PTR)             :: local_err_msg_ptr
        TYPE(C_PTR)             :: local_row_ptr
        TYPE(C_PTR)             :: local_col_ptr
        TYPE(C_PTR)             :: local_data_ptr
        INTEGER(C_INT32_T)      :: local_num_rows_c
        INTEGER(C_INT32_T)      :: local_num_cols_c

        ! Get C pointer and size for the index vector
        c_index_ptr = C_LOC(index_vector)
        c_index_size = INT(SIZE(index_vector), C_INT)

        ! Call the C++ SUBROUTINE via the interface
        ! Pass tag with C null terminator and c_result to be filled
        PRINT *, "Fortran: Calling C++ get_opat_table_c (subroutine) with tag: ", TRIM(tag)
        CALL get_opat_table_c(c_index_ptr, c_index_size, TRIM(tag) // C_NULL_CHAR, c_result)

        ! Copy results from BIND(C) type to local variables FIRST (same as before)
        local_error_code = c_result%errorcode
        local_err_msg_ptr = c_result%errormessageptr
        local_row_ptr = c_result%rowvaluesptr
        local_col_ptr = c_result%columnvaluesptr
        local_data_ptr = c_result%dataptr
        local_num_rows_c = c_result%numrows
        local_num_cols_c = c_result%numcolumns

        PRINT *, "Fortran: Returned from C++ subroutine. Error code: ", local_error_code

        ! Initialize output table error state
        fortran_table%error_code = INT(local_error_code)
        IF (ASSOCIATED(fortran_table%row_values)) NULLIFY(fortran_table%row_values)
        IF (ASSOCIATED(fortran_table%col_values)) NULLIFY(fortran_table%col_values)
        IF (ASSOCIATED(fortran_table%data)) NULLIFY(fortran_table%data)
        fortran_table%num_rows = 0
        fortran_table%num_cols = 0
        IF (ALLOCATED(fortran_table%error_message)) DEALLOCATE(fortran_table%error_message)

        ! Check for errors reported by C++ using local variables
        IF (fortran_table%error_code /= 0) THEN
            IF (C_ASSOCIATED(local_err_msg_ptr)) THEN
                 CALL C_F_STRCPY(local_err_msg_ptr, fortran_table%error_message)
                 PRINT *, "Fortran Error: ", TRIM(fortran_table%error_message)
             ELSE
                 ALLOCATE(CHARACTER(LEN=40) :: fortran_table%error_message) ! Allocate space
                 fortran_table%error_message = "Unknown C++ error (null message pointer)"
                 PRINT *, "Fortran Error: ", TRIM(fortran_table%error_message)
            END IF
             RETURN ! Return early on error
        END IF

        ! If successful, associate Fortran pointers with C pointers
        fortran_table%num_rows = INT(local_num_rows_c)
        fortran_table%num_cols = INT(local_num_cols_c)

        IF (fortran_table%num_rows > 0) THEN
             CALL C_F_POINTER(local_row_ptr, fortran_table%row_values, (/fortran_table%num_rows/))
        END IF
        IF (fortran_table%num_cols > 0) THEN
            CALL C_F_POINTER(local_col_ptr, fortran_table%col_values, (/fortran_table%num_cols/))
        END IF
        IF (fortran_table%num_rows > 0 .AND. fortran_table%num_cols > 0) THEN
             CALL C_F_POINTER(local_data_ptr, fortran_table%data, (/fortran_table%num_rows, fortran_table%num_cols/))
             PRINT *, "Fortran: Successfully associated pointers. Dims:", fortran_table%num_rows, fortran_table%num_cols
        END IF
         ! Ensure error message is empty on success
        IF (ALLOCATED(fortran_table%error_message)) DEALLOCATE(fortran_table%error_message)
        ALLOCATE(CHARACTER(LEN=0) :: fortran_table%error_message)

    END SUBROUTINE get_opat_table


    ! **** MODIFIED C_F_STRCPY ****
    ! Uses TRANSFER and assumes C_CHAR size is 1 byte
    SUBROUTINE C_F_STRCPY(cptr, fstr)
        USE, INTRINSIC :: ISO_C_BINDING
        IMPLICIT NONE
        TYPE(C_PTR), INTENT(IN) :: cptr
        CHARACTER(:), ALLOCATABLE, INTENT(OUT) :: fstr
        ! Local variables
        CHARACTER(KIND=C_CHAR), POINTER :: local_c_char_ptr => NULL()
        INTEGER(C_INTPTR_T) :: current_addr, start_addr
        INTEGER :: length         ! Determined length
        INTEGER :: i              ! Loop index (offset)
        INTEGER, PARAMETER :: buffer_size = 10000 ! Max length including null
        CHARACTER(LEN=buffer_size) :: buffer  ! Temp buffer
        LOGICAL :: found_null

        IF (.NOT. C_ASSOCIATED(cptr)) THEN
            fstr = ""
            RETURN
        ENDIF

        start_addr = TRANSFER(cptr, start_addr) ! Get integer representation of start address
        length = 0
        found_null = .false.

        DO i = 0, buffer_size - 1  ! Iterate offsets 0 to buffer_size-1
            ! Calculate address using assumed C_CHAR size of 1 byte
            current_addr = start_addr + i

            ! Convert address back to C_PTR and associate a Fortran pointer
            CALL C_F_POINTER(TRANSFER(current_addr, C_NULL_PTR), local_c_char_ptr)

            IF (.NOT. ASSOCIATED(local_c_char_ptr)) THEN
                PRINT *, "Warning(C_F_STRCPY): Pointer association failed at offset", i
                length = i
                found_null = .true.
                EXIT
            END IF

            IF (local_c_char_ptr == C_NULL_CHAR) THEN
                length = i
                found_null = .true.
                EXIT
            END IF

             IF (i + 1 <= LEN(buffer)) THEN
                 buffer(i+1 : i+1) = local_c_char_ptr
             ELSE
                 PRINT *, "Error(C_F_STRCPY): Buffer overflow attempt."
                 length = i
                 found_null = .true.
                 EXIT
             END IF
        END DO

        IF (.not. found_null) THEN
             length = buffer_size
             PRINT *, "Warning(C_F_STRCPY): Null terminator not found within buffer limit (", buffer_size, "). String might be truncated."
        END IF

        IF (length > 0) THEN
            ALLOCATE(CHARACTER(LEN=length) :: fstr)
            fstr = buffer(1:length)
        ELSE
            fstr = ""
        ENDIF

    END SUBROUTINE C_F_STRCPY

END MODULE opat_interface_module