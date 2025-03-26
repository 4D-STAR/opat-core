#include <gtest/gtest.h>
#include "opatIO.h"
#include "picosha2.h"

#include <iostream>
#include <string>

std::string EXAMPLE_FILENAME = std::string(getenv("MESON_SOURCE_ROOT")) + "/opatIO-cpp/tests/synthetic_tables.opat";

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
    EXPECT_NO_THROW(OpatIO());
}

/* 
 * @test Verify constructor initializes correctly with a file.
 */
TEST_F(opatIOTest, Constructor) {
    OpatIO opatIO(EXAMPLE_FILENAME);
}

/**
 * @test Verify the header is read correctly.
 */
TEST_F(opatIOTest, Header) {
    OpatIO opatIO(EXAMPLE_FILENAME);
    Header header = opatIO.getHeader();
    EXPECT_EQ(header.version, 1);
    EXPECT_EQ(header.numTables, 20);
    EXPECT_EQ(header.headerSize, 256);
    EXPECT_EQ(header.indexOffset, 416416);
    EXPECT_EQ(std::string(header.creationDate), "Feb 17, 2025");
    EXPECT_EQ(std::string(header.sourceInfo), "utils/opatio/utils/mkTestData.py");
    EXPECT_EQ(std::string(header.comment), "Synthetic Opacity Tables");
    EXPECT_EQ(header.numIndex, 2);
}

/**
 * @test Verify the number of index values is correct. Also check the byte position and index vector
 */
TEST_F(opatIOTest, TableIndex) {
    OpatIO opatIO(EXAMPLE_FILENAME);
    std::vector<TableIndex> tableIndex = opatIO.getTableIndex();
    EXPECT_EQ(tableIndex.size(), 20);
    EXPECT_EQ(tableIndex[0].index.at(0), 0.1);
    EXPECT_EQ(tableIndex[0].index.at(1), 0.001);
    EXPECT_EQ(tableIndex[0].byteStart, 256);
    EXPECT_EQ(tableIndex[0].byteEnd, 21064);
}

/**
 * @test Verify the maxQDepth (for caching)
 */
TEST_F(opatIOTest, MaxQDepth) {
    OpatIO opatIO(EXAMPLE_FILENAME);
    EXPECT_EQ(opatIO.getMaxQDepth(), 20);
    opatIO.setMaxQDepth(5);
    EXPECT_EQ(opatIO.getMaxQDepth(), 5);
}

/**
 * @test Verify the Unload function
 */
TEST_F(opatIOTest, Unload){
    OpatIO opatIO(EXAMPLE_FILENAME);
    EXPECT_NO_THROW(opatIO.unload());
    EXPECT_FALSE(opatIO.isLoaded());
}

/**
 * @test Verify the lookupTableID function
 */
TEST_F(opatIOTest, LookupTableID) {
    OpatIO opatIO(EXAMPLE_FILENAME);
    std::vector<double> index = {0.321053, 0.0116842};
    EXPECT_EQ(opatIO.lookupTableID(index), 7);
}

/**
 * @test Verify the GetTable function by checking the first 2x2 square of the table
 */
TEST_F(opatIOTest, GetTable) {
    OpatIO opatIO(EXAMPLE_FILENAME);
    std::vector<double> index = {0.1, 0.001};
    OPATTable tab = opatIO.getTable(index);
    EXPECT_EQ(tab.N_R, 50);
    EXPECT_EQ(tab.N_T, 50);
    EXPECT_DOUBLE_EQ(tab.logR[0], -8.0);
    EXPECT_DOUBLE_EQ(tab.logT[0], 3.0);
    EXPECT_DOUBLE_EQ(tab.logKappa[0][0], -0.50183952461055);
    EXPECT_DOUBLE_EQ(tab.logKappa[0][1], 1.8028572256396647);
    EXPECT_DOUBLE_EQ(tab.logKappa[1][0], 1.8783385110582342);
    EXPECT_DOUBLE_EQ(tab.logKappa[1][1], 1.1005312934444582);
}



/**
 * @test Verify the GetTable function by computing the checksum of the first table and 
 * comparing it to the stored checksum
 */
TEST_F(opatIOTest, Checksum) {
    OpatIO opatIO(EXAMPLE_FILENAME);
    std::vector<double> index = {0.1, 0.001};
    TableIndex tableIndex = opatIO.getTableIndex(index);
    std::vector<unsigned char> hash = opatIO.computeChecksum(index);
    std::string hexRepr = picosha2::bytes_to_hex_string(hash);

    std::vector<unsigned char> storedHashVec(tableIndex.sha256, tableIndex.sha256 + 32);
    std::string storedHexRepr = picosha2::bytes_to_hex_string(storedHashVec);
    EXPECT_EQ(hexRepr, storedHexRepr);
}

TEST_F(opatIOTest, ChecksumAll) {
    OpatIO opatIO(EXAMPLE_FILENAME);
    opatIO.setMaxQDepth(5);
    EXPECT_TRUE(opatIO.validateAll());
}