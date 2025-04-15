#include <cxxopts.hpp>
#include <iostream>
#include <string>
#include <filesystem>
#include <stdexcept>

#include "opatIO.h"

int main(int argc, char* argv[]) {
    /**
     * @brief Entry point for the OPAT Card Catalog Viewer tool.
     * 
     * This utility allows users to inspect the header and card catalog information
     * of an OPAT file. It uses the cxxopts library to parse command-line arguments
     * and the opatIO library to read and process OPAT files.
     * 
     * Command-line options:
     * - `-f` or `--file`: Specifies the path to the OPAT file to inspect.
     * 
     * Functionality:
     * 1. Parse the command-line arguments to retrieve the file path.
     * 2. Validate the existence and type of the provided file path.
     * 3. If valid, read the OPAT file and display its header and card catalog information.
     * 4. Handle errors such as missing file paths, non-existent files, or invalid file types.
     * 
     * @param argc Number of command-line arguments.
     * @param argv Array of command-line argument strings.
     * @return int Exit code (0 for success, non-zero for errors).
     */

    // Define ANSI color codes for terminal output
    std::string ANSI_COLOR_RED = "\x1b[31m";
    std::string ANSI_COLOR_GREEN = "\x1b[32m";
    std::string ANSI_COLOR_YELLOW = "\x1b[33m";
    std::string ANSI_COLOR_RESET = "\x1b[0m";

    // Configure command-line options using cxxopts
    cxxopts::Options options("OpatIO Card Catalog Viewer", 
                             "Simple utility to view OPAT Header and Card Catalog information");

    options.add_options()
    ("f,file", "File name", cxxopts::value<std::string>());

    // Parse command-line arguments
    auto result = options.parse(argc, argv);

    // Check if the file path is provided
    if (result.count("file")) {
        std::string filePath = result["file"].as<std::string>();

        // Validate the file path
        if (std::filesystem::exists(filePath)) {
            if (std::filesystem::is_regular_file(filePath)) {
                // Read the OPAT file
                opat::OPAT opat = opat::readOPAT(filePath);

                // Display OPAT header information
                std::cout << ANSI_COLOR_GREEN << "== OPAT Header Information ==" 
                          << ANSI_COLOR_RESET << std::endl;
                opat.header.print();

                // Display OPAT table index information
                std::cout << ANSI_COLOR_GREEN << "== OPAT Table Index Information ==" 
                          << ANSI_COLOR_RESET << std::endl;
                for (const auto& entry : opat.cardCatalog.tableIndex) {
                    std::cout << entry.second << std::endl;
                }
            } else {
                // Error: Provided path is not a regular file
                throw std::invalid_argument("The file path provided is not a regular file.");
            }
        } else {
            // Error: File does not exist
            throw std::invalid_argument("The file path provided does not exist.");
        }
    } else {
        // No file path provided
        std::cout << "No file path provided..." << std::endl;
    }
}
