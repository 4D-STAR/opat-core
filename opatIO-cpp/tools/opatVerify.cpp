#include <cxxopts.hpp>
#include <iostream>
#include <string>
#include <filesystem>
#include <stdexcept>

#include "opatIO.h"

int main(int argc, char* argv[]) {
    cxxopts::Options options("OpatIO Header Viewer", "Simple utility to validate if an opat file is valid");

    options.add_options()
    ("f,file", "File name", cxxopts::value<std::string>());

    auto result = options.parse(argc, argv);

    if (result.count("file")) {
        std::string filePath = result["file"].as<std::string>();

        if (std::filesystem::exists(filePath)) {
            if (std::filesystem::is_regular_file(filePath)) {
                try {
                    OpatIO opatIO(filePath);
                    std::cout << "The file is a valid OPAT file." << std::endl;
                } catch (const std::exception &e) {
                    std::cout << "The file is not a valid OPAT file: " << e.what() << std::endl;
                }
            } else {
                throw std::invalid_argument("The file path provided is not a regular file.");
            }
        } else {
            throw std::invalid_argument("The file path provided does not exist.");
        }
    } else {
        std::cout << "No file path provided..." << std::endl;
    }


}