#include <cxxopts.hpp>
#include <iostream>
#include <string>
#include <filesystem>
#include <stdexcept>

#include "opatIO.h"

/**
 * @brief Entry point for the OpatIO Header Viewer utility.
 *
 * This program is a command-line tool designed to read and display the header
 * information of an OPAT file. It uses the `cxxopts` library to parse command-line
 * arguments and the `opatIO` library to handle OPAT file operations.
 *
 * Command-line options:
 * - `-f` or `--file`: Specifies the path to the OPAT file to be read.
 *
 * The program performs the following steps:
 * 1. Parses the command-line arguments to retrieve the file path.
 * 2. Validates the existence and type of the file path provided.
 * 3. Reads the OPAT file and prints its header information.
 *
 * If the file path is invalid or not provided, the program throws an exception
 * or displays an error message.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @return int Exit status of the program (0 for success, non-zero for failure).
 */
int main(int argc, char* argv[]) {
    // Define ANSI color codes for colored output in the terminal.
    std::string ANSI_COLOR_RED = "\x1b[31m";
    std::string ANSI_COLOR_GREEN = "\x1b[32m";
    std::string ANSI_COLOR_YELLOW = "\x1b[33m";
    std::string ANSI_COLOR_RESET = "\x1b[0m";

    // Create a cxxopts::Options object to define and parse command-line options.
    cxxopts::Options options("OpatIO Header Viewer", "Simple utility to view OPAT header information");

    // Add the `--file` option to specify the file path.
    options.add_options()
    ("f,file", "File name", cxxopts::value<std::string>());

    // Parse the command-line arguments.
    auto result = options.parse(argc, argv);

    // Check if the `--file` option was provided.
    if (result.count("file")) {
        std::string filePath = result["file"].as<std::string>();

        // Validate that the file exists and is a regular file.
        if (std::filesystem::exists(filePath)) {
            if (std::filesystem::is_regular_file(filePath)) {
                // Read the OPAT file and print its header information.
                opat::OPAT opat = opat::readOPAT(filePath);
                opat.header.print();
            } else {
                // Throw an error if the file path is not a regular file.
                throw std::invalid_argument(ANSI_COLOR_RED + "The file path provided is not a regular file." + ANSI_COLOR_RESET);
            }
        } else {
            // Throw an error if the file does not exist.
            throw std::invalid_argument(ANSI_COLOR_RED + "The file path provided does not exist." + ANSI_COLOR_RESET);
        }
    } else {
        // Display a message if no file path was provided.
        std::cout << "No file path provided (Note that you must provide file paths as a flag, i.e. opatHeader -f <path/to/file>)..." << std::endl;
    }
}
