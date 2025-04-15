/**
 * @file fextern.cpp
 * @brief Provides C-style API for interacting with OPAT files.
 * 
 * This file contains functions to load, free, and retrieve data from OPAT files
 * using a C-style interface. It is designed to be used in environments where
 * C++ code needs to be accessed from C or other languages.
 */

#include "opatIO.h"
#include "indexVector.h"
#include <vector>
#include <cstring>
#include <iostream>
#include <string>

extern "C" {

    /**
     * @struct OPATTable_C
     * @brief Represents a table structure in an OPAT file.
     * 
     * This structure is used to return table data from an OPAT file.
     */
    struct OPATTable_C {
        double* rowValuesPtr;    /**< Pointer to the row values of the table. */
        double* colValuesPtr;    /**< Pointer to the column values of the table. */
        double* dataPtr;         /**< Pointer to the data values of the table. */
        int32_t numRows;         /**< Number of rows in the table. */
        int32_t numCols;         /**< Number of columns in the table. */
        int errorCode;           /**< Error code (0 for success, non-zero for errors). */
        const char* errorMessage;/**< Error message in case of failure. */
    };

    static opat::OPAT* loaded_opat_file = nullptr; /**< Pointer to the currently loaded OPAT file. */
    static std::string last_loaded_filename = ""; /**< Name of the last successfully loaded OPAT file. */
    static std::string last_error_message = "";   /**< Last error message encountered. */

    /**
     * @brief Loads an OPAT file.
     * 
     * This function loads an OPAT file into memory. If a file is already loaded,
     * it will be unloaded before loading the new file.
     * 
     * @param filename The path to the OPAT file to load.
     * @return 0 on success, -1 on failure.
     * 
     * @example
     * @code
     * int result = load_opat_file_c("example.opat");
     * if (result == 0) {
     *     // File loaded successfully
     * } else {
     *     // Handle error
     * }
     * @endcode
     */
    int load_opat_file_c(const char* filename) {
        if (loaded_opat_file != nullptr && last_loaded_filename == filename) {
            return 0; // File already loaded
        }
        if (loaded_opat_file != nullptr) {
            delete loaded_opat_file;
            loaded_opat_file = nullptr;
        }

        last_loaded_filename = "";

        try {
            std::cout << "C++: Loading OPAT file: " << filename << std::endl;
            loaded_opat_file = new opat::OPAT(opat::readOPAT(filename));
            last_loaded_filename = filename;
            std::cout << "C++: Loaded OPAT file successfully." << std::endl;
            return 0; // Success
        } catch (const std::exception& e) {
            delete loaded_opat_file;
            loaded_opat_file = nullptr;

            last_error_message = e.what();
            std::cerr << "C++: Error loading OPAT file: " << last_error_message << std::endl;
            return -1; // Error
        }
    }

    /**
     * @brief Frees the currently loaded OPAT file.
     * 
     * This function unloads the currently loaded OPAT file and releases any
     * associated resources.
     * 
     * @example
     * @code
     * free_opat_file_c();
     * @endcode
     */
    void free_opat_file_c() {
        if (loaded_opat_file != nullptr) {
            delete loaded_opat_file;
            loaded_opat_file = nullptr;
            last_loaded_filename = "";
            last_error_message = "";
        }
    }

    /**
     * @brief Retrieves a table from the loaded OPAT file.
     * 
     * This function retrieves a table from the currently loaded OPAT file based
     * on the provided index vector and table tag. The result is stored in the
     * provided `OPATTable_C` structure.
     * 
     * @param index_vector_ptr Pointer to the index vector.
     * @param index_vector_size Size of the index vector.
     * @param table_tag The tag identifying the table to retrieve.
     * @param result_out Pointer to an `OPATTable_C` structure to store the result.
     * 
     * @example
     * @code
     * double index_vector[] = {1.0, 2.0, 3.0};
     * OPATTable_C result;
     * get_opat_table_c(index_vector, 3, "example_table", &result);
     * if (result.errorCode == 0) {
     *     // Access table data using result.rowValuesPtr, result.colValuesPtr, and result.dataPtr
     * } else {
     *     // Handle error
     * }
     * @endcode
     */
    void get_opat_table_c(const double* index_vector_ptr, int index_vector_size, const char* table_tag, OPATTable_C* result_out) {
        result_out->rowValuesPtr = nullptr;
        result_out->colValuesPtr = nullptr;
        result_out->dataPtr = nullptr;
        result_out->numRows = 0;
        result_out->numCols = 0;
        result_out->errorCode = 1; // Default error code
        result_out->errorMessage = "OPAT file not loaded";
        std::cout << "C++: get_opat_table_c called" << std::endl;
        if (!loaded_opat_file) {
            result_out->errorCode = -1;
            result_out->errorMessage = "OPAT FILE NOT LOADED. Call load_opat_file_c() first.";
        }


        try {
            std::string table_tag_str(table_tag);
            std::vector<double> index_vec(index_vector_ptr, index_vector_ptr + index_vector_size);
            FloatIndexVector fiv(index_vec, 8);
            const std::string tag(table_tag_str);

            const auto& card = loaded_opat_file->get(fiv);
            const auto& table = card.get(tag);

            result_out->rowValuesPtr = table.rowValues.get();
            result_out->colValuesPtr = table.columnValues.get();

            result_out->dataPtr = table.data.get();

            result_out->numRows = static_cast<int32_t>(table.N_R);
            result_out->numCols = static_cast<int32_t>(table.N_C);

            result_out->errorCode = 0;
            result_out->errorMessage = "";
        } catch (const std::out_of_range& e) {
            result_out->errorCode = -1;
            result_out->errorMessage = "Table not found in OPAT file.";
        } catch (const std::exception& e) {
            std::cerr << "C++: Error in get_opat_table_c: " << e.what() << std::endl;
            result_out->errorCode = -1;
            result_out->errorMessage = e.what();
        }
    }

} // extern "C"
