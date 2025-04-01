#include <cxxopts.hpp>
#include <iostream>
#include <string>
#include <filesystem>
#include <stdexcept>

#include "opatIO.h"


int main(int argc, char* argv[]) {
    std::string ANSI_COLOR_RED = "\x1b[31m";
    std::string ANSI_COLOR_GREEN = "\x1b[32m";
    std::string ANSI_COLOR_YELLOW = "\x1b[33m";
    std::string ANSI_COLOR_RESET = "\x1b[0m";

    cxxopts::Options options("OpatIO Header Viewer", "Simple utility to view OPAT header information");

    options.add_options()
    ("f,file", "File name", cxxopts::value<std::string>());

    auto result = options.parse(argc, argv);

    if (result.count("file")) {
        std::string filePath = result["file"].as<std::string>();

        if (std::filesystem::exists(filePath)) {
            if (std::filesystem::is_regular_file(filePath)) {
                opat::OPAT opat = opat::readOPAT(filePath);
                opat.header.print();
            } else {
                throw std::invalid_argument(ANSI_COLOR_RED + "The file path provided is not a regular file."+ANSI_COLOR_RESET);
            }
        } else {
            throw std::invalid_argument(ANSI_COLOR_RED+"The file path provided does not exist."+ANSI_COLOR_RESET);
        }
    } else {
        std::cout << "No file path provided..." << std::endl;
    }


}