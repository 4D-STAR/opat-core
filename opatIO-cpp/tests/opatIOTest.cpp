#include <gtest/gtest.h>
#include "opatIO.h"
#include "indexVector.h"
#include "picosha2.h"

#include <iostream>
#include <string>

std::string EXAMPLE_FILENAME = std::string(getenv("MESON_SOURCE_ROOT")) + "/opatIO-cpp/tests/gs98hz.opat";

/**
 * @file opatIOTest.cpp
 * @brief Unit tests for the OpatIO class and associated structs.
 */

/**
 * @brief Test suite for the const class.
 */
class opatIOTest : public ::testing::Test {};

/**
 * @test Verify default constructor initializes correctly.
 */
TEST_F(opatIOTest, DefaultConstructor) {
    opat::OPAT opat = opat::readOPAT(EXAMPLE_FILENAME);
    FloatIndexVector index({0.35, 0.004});
    // const opat::DataCard& card = opat.get(index);
    std::cout << opat[index]["data"] << std::endl;
    std::cout << opat[index]["data"](5, 35) << std::endl;
    std::cout << opat[index]["data"].getRow(5) << std::endl;
    std::cout << opat[index]["data"].getColumn(35) << std::endl;

    opat::Slice rowSlice(0, 6);
    opat::Slice colSlice(25, 36);

    std::cout << opat[index]["data"].slice(rowSlice, colSlice).ascii() << std::endl;
    std::cout << (opat[index]["data"].slice(rowSlice, colSlice)(1, 1)) << std::endl;

    opat[index]["data"].slice(rowSlice, colSlice).print();

    std::cout << opat[index]["data"].slice(rowSlice, colSlice).size().first << std::endl;
    std::cout << opat[index]["data"].slice(rowSlice, colSlice).size().second << std::endl;

    std::cout << opat[index].header << std::endl;
    std::cout << opat[index].tableIndex["data"].columnName << std::endl;

    std::cout << opat[index]["data"].getColumnValues().ascii() << std::endl;
}
