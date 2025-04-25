#include <cxxopts.hpp>
#include <iostream>
#include <string>
#include <filesystem>
#include <stdexcept>

#include "opatIO.h"

int main(int argc, char* argv[]) {
    /**
     * @brief Main function for the OPAT file validity verifier.
     *
     * This utility is designed to validate whether a given file is a valid OPAT file.
     * It uses the cxxopts library to parse command-line arguments and the opatIO library
     * to read and validate the OPAT file format.
     *
     * Steps:
     * 1. Parse command-line arguments to retrieve the file path.
     * 2. Check if the file path exists and is a regular file.
     * 3. Attempt to read the file as an OPAT file using the opatIO library.
     * 4. Print the result of the validation to the console.
     *
     * Command-line arguments:
     * - `--file` or `-f`: Specifies the path to the file to be validated.
     *
     * Error handling:
     * - If the file path does not exist or is not a regular file, an exception is thrown.
     * - If the file cannot be read as a valid OPAT file, an error message is displayed.
     *
     * @param argc Number of command-line arguments.
     * @param argv Array of command-line arguments.
     * @return int Exit code (0 for success, non-zero for failure).
     */
    cxxopts::Options options("OpatIO Validity Verifier", "Simple utility to validate if an opat file is valid");

    // Define the command-line options
    options.add_options()
    ("f,file", "File name", cxxopts::value<std::string>());

    // Parse the command-line arguments
    auto result = options.parse(argc, argv);

    // Check if the "file" option was provided
    if (result.count("file")) {
        std::string filePath = result["file"].as<std::string>();

        // Verify if the file path exists and is a regular file
        if (std::filesystem::exists(filePath)) {
            if (std::filesystem::is_regular_file(filePath)) {
                try {
                    // Attempt to read the file as an OPAT file
                    opat::OPAT opat = opat::readOPAT(filePath);
                    std::cout << "The file is a valid OPAT file." << std::endl;
                } catch (const std::exception &e) {
                    // Handle errors during OPAT file reading
                    std::cout << "The file is not a valid OPAT file: " << e.what() << std::endl;
                }
            } else {
                // Error: The provided path is not a regular file
                throw std::invalid_argument("The file path provided is not a regular file.");
            }
        } else {
            // Error: The provided path does not exist
            throw std::invalid_argument("The file path provided does not exist.");
        }
    } else {
        // No file path was provided in the command-line arguments
        std::cout << "No file path provided (Note that you must provide file paths as a flag, i.e. opatVerify -f <path/to/file>)..." << std::endl;
    }
}
