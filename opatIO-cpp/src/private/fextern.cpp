#include "opatIO.h"
#include "indexVector.h"
#include <vector>
#include <cstring>
#include <iostream>
#include <string>

extern "C" {
    struct OPATTable_C {
        double* rowValuesPtr;
        double* colValuesPtr;
        double* dataPtr;
        int32_t numRows;
        int32_t numCols;

        int errorCode;
        const char* errorMessage;
    };

    static opat::OPAT* loaded_opat_file = nullptr;
    static std::string last_loaded_filename = "";
    static std::string last_error_message = "";

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

    void free_opat_file_c() {
        if (loaded_opat_file != nullptr) {
            delete loaded_opat_file;
            loaded_opat_file = nullptr;
            last_loaded_filename = "";
            last_error_message = "";
        }
    }

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