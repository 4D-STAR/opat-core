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
    EXPECT_NO_THROW(opat::readOPAT(EXAMPLE_FILENAME));
}

/**
 * @test Verify reading the header of an OPAT file.
 */
TEST_F(opatIOTest, ReadHeader) {
    std::ifstream file(EXAMPLE_FILENAME, std::ios::binary);
    ASSERT_TRUE(file.is_open());
    opat::Header header = opat::readHeader(file);
    ASSERT_EQ(header.version, 1); // Replace -1 with the expected version
    ASSERT_EQ(header.numTables, 126); // Replace -1 with the expected number of tables
    file.close();
}

/**
 * @test Verify reading the CardCatalog of an OPAT file.
 */
TEST_F(opatIOTest, ReadCardCatalog) {
    std::ifstream file(EXAMPLE_FILENAME, std::ios::binary);
    ASSERT_TRUE(file.is_open());
    opat::Header header = opat::readHeader(file);
    opat::CardCatalog catalog = opat::readCardCatalog(file, header);
    ASSERT_EQ(catalog.tableIndex.size(), 126); // Replace -1 with the expected size
    file.close();
}

/**
 * @test Verify reading a DataCard from an OPAT file.
 */
TEST_F(opatIOTest, ReadDataCard) {
    std::ifstream file(EXAMPLE_FILENAME, std::ios::binary);
    ASSERT_TRUE(file.is_open());
    opat::Header header = opat::readHeader(file);
    opat::CardCatalog catalog = opat::readCardCatalog(file, header);
    auto firstEntry = catalog.tableIndex.begin();
    opat::DataCard card = opat::readDataCard(file, firstEntry->second);
    ASSERT_EQ(card.header.numTables, 1); // Replace -1 with the expected number of tables
    file.close();
}

/**
 * @test Verify accessing a table by tag in a DataCard.
 */
TEST_F(opatIOTest, AccessTableByTag) {
    opat::OPAT opat = opat::readOPAT(EXAMPLE_FILENAME);
    FloatIndexVector index({0.35, 0.004});
    const auto& table = opat[index]["data"];
    ASSERT_EQ(table.size().first, 19); // Replace -1 with the expected number of rows
    ASSERT_EQ(table.size().second, 70); // Replace -1 with the expected number of columns
}

/**
 * @test Verify slicing a table.
 */
TEST_F(opatIOTest, SliceTable) {
    opat::OPAT opat = opat::readOPAT(EXAMPLE_FILENAME);
    FloatIndexVector index({0.35, 0.004});
    opat::Slice rowSlice(0, 6);
    opat::Slice colSlice(25, 36);
    auto slicedTable = opat[index]["data"].slice(rowSlice, colSlice);
    ASSERT_EQ(slicedTable.size().first, 6); // Replace -1 with the expected sliced rows
    ASSERT_EQ(slicedTable.size().second, 11); // Replace -1 with the expected sliced columns
}

/**
 * @test Verify retrieving a single row from a table.
 */
TEST_F(opatIOTest, GetRow) {
    opat::OPAT opat = opat::readOPAT(EXAMPLE_FILENAME);
    FloatIndexVector index({0.35, 0.004});
    auto row = opat[index]["data"].getRow(5);
    ASSERT_EQ(row.size().second, 70); // Replace -1 with the expected number of columns in the row
}

/**
 * @test Verify retrieving a single column from a table.
 */
TEST_F(opatIOTest, GetColumn) {
    opat::OPAT opat = opat::readOPAT(EXAMPLE_FILENAME);
    FloatIndexVector index({0.35, 0.004});
    auto column = opat[index]["data"].getColumn(35);
    ASSERT_EQ(column.size().first, 19); // Replace -1 with the expected number of rows in the column
}

/**
 * @test Verify retrieving raw data from a table.
 */
TEST_F(opatIOTest, GetRawData) {
    opat::OPAT opat = opat::readOPAT(EXAMPLE_FILENAME);
    FloatIndexVector index({0.35, 0.004});
    const double* rawData = opat[index]["data"].getRawData();
    ASSERT_NE(rawData, nullptr);
}

/**
 * @test Verify ASCII representation of a table.
 */
TEST_F(opatIOTest, TableAscii) {
    opat::OPAT opat = opat::readOPAT(EXAMPLE_FILENAME);
    FloatIndexVector index({0.35, 0.004});
    std::string ascii = opat[index]["data"].ascii();
    ASSERT_FALSE(ascii.empty());
}

TEST_F(opatIOTest, getValue) {
    opat::OPAT opat = opat::readOPAT(EXAMPLE_FILENAME);
    FloatIndexVector index({0.35, 0.004});
    const double& value = opat[index]["data"](5, 35, 0);
    EXPECT_DOUBLE_EQ(value, -0.402); // Replace -1 with the expected value
}

TEST_F(opatIOTest, getBounds) {
    opat::OPAT opat = opat::readOPAT(EXAMPLE_FILENAME);
    FloatIndexVector index({0.35, 0.004});
    const auto bounds = opat.getBounds();
    EXPECT_EQ(bounds.size(), 2);
    EXPECT_DOUBLE_EQ(bounds.at(0).min, 0);
    EXPECT_DOUBLE_EQ(bounds.at(0).max, 1);
    EXPECT_DOUBLE_EQ(bounds.at(1).min, 0);
    EXPECT_DOUBLE_EQ(bounds.at(1).max, 0.1);
}
