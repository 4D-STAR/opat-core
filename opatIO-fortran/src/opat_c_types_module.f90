! opat_c_types_module.f90
MODULE opat_c_types_module
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE

    PRIVATE ! Only expose the type

    ! Define Fortran TYPE mirroring the C struct OPATTable_C
    TYPE, BIND(C), PUBLIC :: opat_table_c_type ! Make the type PUBLIC
        TYPE(C_PTR) :: rowvaluesptr = C_NULL_PTR
        TYPE(C_PTR) :: columnvaluesptr = C_NULL_PTR
        TYPE(C_PTR) :: dataptr = C_NULL_PTR
        INTEGER(C_INT32_T) :: numrows = 0
        INTEGER(C_INT32_T) :: numcolumns = 0
        INTEGER(C_INT)     :: errorcode = 1 ! Default to error state
        TYPE(C_PTR) :: errormessageptr = C_NULL_PTR ! Pointer to C string
    END TYPE opat_table_c_type

END MODULE opat_c_types_module