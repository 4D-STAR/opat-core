/* ***********************************************************************
//
//   Copyright (C) 2025 -- The 4D-STAR Collaboration
//   File Author: Emily Boudreaux
//   Last Modified: March 07, 2025
//
//   4DSSE is free software; you can use it and/or modify
//   it under the terms and restrictions the GNU General Library Public
//   License version 3 (GPLv3) as published by the Free Software Foundation.
//
//   4DSSE is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//   See the GNU Library General Public License for more details.
//
//   You should have received a copy of the GNU Library General Public License
//   along with this software; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
// *********************************************************************** */

/**
 * @file opatIO.h
 * @brief Header file for the OPAT I/O library, providing structures and functions for reading and manipulating OPAT files.
 *
 * This file defines the core data structures and functions used for handling OPAT files in C++.
 * The OPAT format is designed for efficient storage and retrieval of multidimensional data tables,
 * with metadata and indexing for fast access.
 *
 * The design prioritizes:
 * - **Compactness**: Structures are packed to minimize memory usage.
 * - **Flexibility**: Support for multiple tables and metadata.
 * - **Performance**: Use of unordered maps for fast lookups.
 *
 * Usage Example:
 * @code
 * #include "opatIO.h"
 * 
 * int main() {
 *     std::string filename = "example.opat";
 *     opat::OPAT file = opat::readOPAT(filename);
 *     const auto& table = file.get({1.0, 2.0, 3.0}).get("table_tag");
 *     table.print();
 *     return 0;
 * }
 * @endcode
 */

#ifndef OPATIO_H
#define OPATIO_H

#include <fstream>
#include <memory>
#include <string>
#include <sys/types.h>
#include <vector>
#include <utility>
#include <cstdint>
#include <unordered_map>
#include <limits>

#include "indexVector.h"

namespace opat {

/**
 * @brief Structure to hold the header information of an OPAT file.
 *
 * The header contains metadata about the file, such as its version, creation date, and offsets to the card catalog and start of the data section.
 * It is packed to ensure compactness and alignment with the binary file format.
 */
#pragma pack(1)
struct Header {
    char magic[4];           ///< Magic number to identify the file type.
    uint16_t version;        ///< Version of the OPAT file format.
    uint32_t numTables;      ///< Number of tables in the file.
    uint32_t headerSize;     ///< Size of the header in bytes.
    uint64_t indexOffset;    ///< Offset to the index section.
    char creationDate[16];   ///< Creation date of the file (e.g., "YYYY-MM-DD HH:MM").
    char sourceInfo[64];     ///< Source information (e.g., software version or author).
    char comment[128];       ///< User-defined comment section.
    uint16_t numIndex;       ///< Size of the index vector per table.
    uint8_t hashPrecision;  ///< Precision of the hash used for table validation.
    char reserved[23];       ///< Reserved for future use.

    /**
     * @brief Stream insertion operator for printing the header.
     * @param os Output stream.
     * @param header Header to print.
     * @return Reference to the output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const Header& header);

    void print() const; ///< Prints the header information to the console.
};
#pragma pack()

/**
 * @brief Structure to hold the header information of a DataCard in an OPAT file.
 *
 * The CardHeader contains metadata about a specific DataCard, such as its size and offsets.
 * It is packed to ensure alignment with the binary file format.
 */
#pragma pack(1)
struct CardHeader {
    char magic[4];           ///< Magic number to identify the card type.
    uint32_t numTables;      ///< Number of tables in the card.
    uint32_t headerSize;     ///< Size of the card header in bytes.
    uint64_t indexOffset;    ///< Offset to the index section within the card.
    uint64_t cardSize;       ///< Total size of the card in bytes.
    char comment[128];       ///< User-defined comment section.
    char reserved[100];      ///< Reserved for future use.

    /**
     * @brief Stream insertion operator for printing the card header.
     * @param os Output stream.
     * @param header CardHeader to print.
     * @return Reference to the output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const CardHeader& header);
};
#pragma pack()

/**
 * @brief Structure to hold the index information of a table in an OPAT file.
 *
 * Each CardCatalogEntry represents a single table's metadata, including its location in the file
 * and a hash for data integrity verification.
 */
struct CardCatalogEntry {
    FloatIndexVector index;  ///< Index vector for the associated table.
    uint64_t byteStart;      ///< Byte start position of the table in the file.
    uint64_t byteEnd;        ///< Byte end position of the table in the file.
    char sha256[32];         ///< SHA-256 hash of the table data for integrity verification.

    /**
     * @brief Stream insertion operator for printing the catalog entry.
     * @param os Output stream.
     * @param entry CardCatalogEntry to print.
     * @return Reference to the output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const CardCatalogEntry& entry);
};

/**
 * @brief Structure to hold the catalog of tables in a DataCard.
 *
 * The CardCatalog maps index vectors to their corresponding CardCatalogEntry, enabling fast lookups.
 */
struct CardCatalog {
    std::unordered_map<FloatIndexVector, CardCatalogEntry> tableIndex; ///< Map of index vectors to catalog entries.

    /**
     * @brief Stream insertion operator for printing the card catalog.
     * @param os Output stream.
     * @param catalog CardCatalog to print.
     * @return Reference to the output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const CardCatalog& catalog);
};

#pragma pack(1)
/**
 * @brief Structure to hold the index information of a table within a DataCard.
 *
 * Each TableIndexEntry contains metadata about a table, such as its dimensions and location.
 */
struct TableIndexEntry {
    char tag[8];             ///< Tag identifying the table.
    uint64_t byteStart;      ///< Byte start position of the table in the card.
    uint64_t byteEnd;        ///< Byte end position of the table in the card.
    uint16_t numColumns;     ///< Number of columns in the table.
    uint16_t numRows;        ///< Number of rows in the table.
    char columnName[8];      ///< Name of the columns (optional).
    char rowName[8];         ///< Name of the rows (optional).
    uint64_t size;           ///< Vector size of each cell
    char reserved[12];       ///< Reserved for future use.

    /**
     * @brief Stream insertion operator for printing the table index entry.
     * @param os Output stream.
     * @param entry TableIndexEntry to print.
     * @return Reference to the output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const TableIndexEntry& entry);
};
#pragma pack()

/**
 * @brief Structure to hold the index of tables within a DataCard.
 *
 * The TableIndex maps table tags to their corresponding TableIndexEntry, enabling fast lookups within one datacard.
 */
struct TableIndex {
    std::unordered_map<std::string, TableIndexEntry> tableIndex; ///< Map of table tags to index entries.

    /**
     * @brief Stream insertion operator for printing the table index.
     * @param os Output stream.
     * @param index TableIndex to print.
     * @return Reference to the output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const TableIndex& index);

    /**
     * @brief Retrieves a TableIndexEntry by tag.
     * @param tag The tag of the table to retrieve.
     * @return A constant reference to the TableIndexEntry.
     * @throws std::out_of_range if the tag is not found.
     */
    [[nodiscard]] const TableIndexEntry& get(const std::string& tag) const;

    /**
     * @brief Accesses a TableIndexEntry by tag.
     * @param tag The tag of the table to access.
     * @return A constant reference to the TableIndexEntry.
     * @throws std::out_of_range if the tag is not found.
     */
    const TableIndexEntry& operator[](const std::string& tag) const;
};

/**
 * @brief Structure to represent a slice of data.
 *
 * A Slice defines a range of rows or columns to extract from a table.
 *
 * @note This Slice does not support defining step sizes, just ranges.
 *
 * @code
 * // Assume you have an opat file loaded into the variable opat
 * Slice rowSlice(6, 12);
 * Slice colSlice(1, 3);
 *
 * FloatIndexVector index({0.35, 0.004});
 * opat::OPATTable table = opat[index]["data"].slice(rowSlice, colSlice);
 * @endcode
 */
struct Slice {
    uint32_t start; ///< Start index of the slice.
    uint32_t end;   ///< End index of the slice.

    /**
     * @brief Stream insertion operator for printing the slice.
     * @param os Output stream.
     * @param slice Slice to print.
     * @return Reference to the output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const Slice& slice);
};


/**
 * @brief Structure to hold the data of an OPAT table.
 *
 * An OPATTable contains the raw data of a table, along with its row and column values.
 * It provides methods for accessing and slicing the data.
 */
struct OPATTable {
    std::unique_ptr<double[]> rowValues; ///< Array of row values.
    std::unique_ptr<double[]> columnValues; ///< Array of column values.
    std::unique_ptr<double[]> data; ///< Array of table data.

    uint32_t N_R;   ///< Number of rows in the table.
    uint32_t N_C;   ///< Number of columns in the table.
    uint64_t m_vsize; ///< Vector size of each cell

    /**
     * @brief Returns the size of the table as a pair of rows and columns.
     * @return A pair containing the number of rows and columns.
     */
    [[nodiscard]] std::pair<double, double> size() const { return std::make_pair(N_R, N_C); }

    /**
     * @brief Retrieves the vector size of each cell in the table.
     * 
     * The vector size represents the number of elements stored in each cell of the table.
     * 
     * @return The vector size of each cell.
     */
    [[nodiscard]] int vsize() const { return static_cast<int>(m_vsize); }

    /**
     * @brief Accesses the first value in the table's data array.
     * 
     * This operator provides a convenient way to retrieve the first value in the table's data array.
     * It is useful for quick access to the initial element of the table.
     * 
     * @return A constant reference to the first value in the data array.
     * @throws std::runtime_error if the data array is not initialized.
     */
    const double& operator()() const;

    /**
     * @brief Accesses a table value by row and column.
     * @param row The row index.
     * @param column The column index.
     * @return A opat table representing that vector.
     * @throws std::out_of_range if the row or column index is out of bounds.
     *
     * @note The return type of this method will always be a OPATTable even if there is only one value stored in the cell. Use the () operator to get that value as a double or use .get(row, col, 0) to get the primitive numeric representation.
     */
    OPATTable operator()(uint32_t row, uint32_t column) const;

    /**
     * @brief Accesses a table value by row and column.
     * @param row The row index.
     * @param column The column index.
     * @param zdepth The vector index to retrieve
     * @return A constant reference to the value at the specified row and column.
     * @throws std::out_of_range if the row or column index is out of bounds.
     */
    double operator()(uint32_t row, uint32_t column, uint64_t zdepth) const;

    /**
     * @brief Slices the table into a smaller OPATTable.
     * @param rowSlice The range of rows to extract.
     * @param colSlice The range of columns to extract.
     * @return An OPATTable containing the specified slice.
     * @throws std::out_of_range if the slice indices are out of bounds.
     */
    OPATTable operator()(const Slice& rowSlice, const Slice& colSlice) const;

    /**
     * @brief Retrieves a table value by row and column.
     * @param row The row index.
     * @param column The column index.
     * @return A OPATTable representing that vector.
     * @throws std::out_of_range if the row or column index is out of bounds.
     *
     * @note The return type of this method will always be a OPATTable even if there is only one value stored in the cell. Use the () operator to get that value as a double or use .get(row, col, 0) to get the primitive numeric representation.
     */
    [[nodiscard]] OPATTable getData(uint32_t row, uint32_t column) const;

    /**
     * @brief Retrieves a table value by row and column.
     * @param row The row index.
     * @param column The column index.
     * @param zdepth The vector index to retrieve
     * @return A constant reference to the value at the specified row and column.
     * @throws std::out_of_range if the row or column index is out of bounds.
     */
    [[nodiscard]] double getData(uint32_t row, uint32_t column, uint64_t zdepth) const;

    /**
     * @brief Extracts a single row from the table.
     * @param row The row index to extract.
     * @return An OPATTable containing the specified row.
     * @throws std::out_of_range if the row index is out of bounds.
     */
    [[nodiscard]] OPATTable getRow(uint32_t row) const;

    /**
     * @brief Extracts a single column from the table.
     * @param column The column index to extract.
     * @return An OPATTable containing the specified column.
     * @throws std::out_of_range if the column index is out of bounds.
     */
    [[nodiscard]] OPATTable getColumn(uint32_t column) const;

    /**
     * @brief Retrieves all row values of the table.
     * @return An OPATTable containing the row values.
     */
    [[nodiscard]] OPATTable getRowValues() const;

    /**
     * @brief Retrieves all column values of the table.
     * @return An OPATTable containing the column values.
     */
    [[nodiscard]] OPATTable getColumnValues() const;

    /**
     * @brief Retrieves the raw data of the table.
     * @return A pointer to the raw data array.
     *
     * @note Using this method opens you up to memory leaks, and it should generally not be used.
     */
    [[nodiscard]] const double* getRawData() const;

    /**
     * @brief Extracts a slice of the table.
     * @param rowSlice The range of rows to extract.
     * @param colSlice The range of columns to extract.
     * @return An OPATTable containing the specified slice.
     * @throws std::out_of_range if the slice indices are out of bounds.
     */
    [[nodiscard]] OPATTable slice(const Slice& rowSlice, const Slice& colSlice) const;

    /**
     * @brief Converts the table to an ASCII representation.
     * @return A string containing the ASCII representation of the table.
     */
    [[nodiscard]] std::string ascii() const;

    /**
     * @brief Prints the table to the standard output.
     */
    void print() const;

    friend std::ostream& operator<<(std::ostream& os, const OPATTable& table);
};

/**
 * @brief Structure to hold a DataCard, which contains multiple tables.
 *
 * A DataCard includes metadata, a table index, and the actual table data.
 */
struct DataCard {
    CardHeader header; ///< Header of the DataCard.
    TableIndex tableIndex; ///< Index of tables within the DataCard.
    std::unordered_map<std::string, OPATTable> tableData; ///< Map of table tags to their data.

    /**
     * @brief Stream insertion operator for printing the DataCard.
     * @param os Output stream.
     * @param card DataCard to print.
     * @return Reference to the output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const DataCard& card);

    /**
     * @brief Retrieves a table from the DataCard by tag.
     * @param tag The tag of the table to retrieve.
     * @return A constant reference to the OPATTable.
     * @throws std::out_of_range if the tag is not found.
     */
    [[nodiscard]] const OPATTable& get(const std::string& tag) const;

    /**
     * @brief Accesses a table from the DataCard by tag.
     * @param tag The tag of the table to access.
     * @return A constant reference to the OPATTable.
     * @throws std::out_of_range if the tag is not found.
     */
    const OPATTable& operator[](const std::string& tag) const;

    /**
     * @brief Accesses a table from the DataCard by tag (C-string version).
     * @param tag The tag of the table to access.
     * @return A constant reference to the OPATTable.
     * @throws std::out_of_range if the tag is not found.
     */
    const OPATTable& operator[](const char* tag) const;

    /**
     * @brief Accesses a table from the DataCard by tag (string_view version).
     * @param tag The tag of the table to access.
     * @return A constant reference to the OPATTable.
     * @throws std::out_of_range if the tag is not found.
     */
    const OPATTable& operator[](std::string_view tag) const;

    /**
     * @brief Retrieves a list of all table tags (keys) present in this DataCard.
     * @return A vector of strings, where each string is a table tag.
     * @example
     * @code
     * // Assuming 'data_card' is an initialized opat::DataCard object
     * std::vector<std::string> table_keys = data_card.getKeys();
     * for (const std::string& key : table_keys) {
     *     std::cout << "Table found: " << key << std::endl;
     *     const opat::OPATTable& table = data_card[key];
     *     // Process the table
     * }
     * @endcode
     */
    [[nodiscard]] std::vector<std::string> getKeys() const;
};

/**
 * @brief Structure to hold the minimum and maximum values for a dimension.
 *
 * This is typically used to represent the bounds of index vectors in an OPAT file.
 *
 * @example
 * @code
 * opat::Bounds dim_bounds;
 * dim_bounds.min = 0.0;
 * dim_bounds.max = 10.0;
 * std::cout << "Dimension bounds: " << dim_bounds << std::endl; // Output: Bounds(0, 10)
 * @endcode
 */
struct Bounds {
    double min = std::numeric_limits<double>::max(); ///< The minimum value.
    double max = std::numeric_limits<double>::min(); ///< The maximum value.
    /**
     * @brief Stream insertion operator for printing the Bounds.
     * @param os Output stream.
     * @param bounds Bounds to print.
     * @return Reference to the output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const Bounds& bounds);
};

/**
 * @brief Structure to hold the entire OPAT file.
 *
 * The OPAT structure contains the file header, card catalog, and all DataCards.
 */
struct OPAT {
    Header header; ///< Header of the OPAT file.
    CardCatalog cardCatalog; ///< Catalog of DataCards in the file.
    std::unordered_map<FloatIndexVector, DataCard> cards; ///< Map of index vectors to DataCards.

    /**
     * @brief Stream insertion operator for printing the OPAT structure.
     * @param os Output stream.
     * @param opat OPAT structure to print.
     * @return Reference to the output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const OPAT& opat);

    /**
     * @brief Retrieves a DataCard from the OPAT structure by index.
     * @param index The index vector of the DataCard to retrieve.
     * @return A constant reference to the DataCard.
     * @throws std::out_of_range if the index is not found.
     */
    [[nodiscard]] const DataCard& get(const FloatIndexVector& index) const;

    /**
     * @brief Retrieves a DataCard from the OPAT structure by a standard vector of doubles.
     * This is a convenience overload that constructs a FloatIndexVector internally.
     * @param index The std::vector<double> representing the index of the DataCard to retrieve.
     * @return A constant reference to the DataCard.
     * @throws std::out_of_range if the index is not found.
     * @example
     * @code
     * // Assuming 'opat_file' is an initialized opat::OPAT object
     * std::vector<double> my_index = {1.0, 2.5};
     * try {
     *     const opat::DataCard& card = opat_file.get(my_index);
     *     // Use the card
     * } catch (const std::out_of_range& e) {
     *     std::cerr << "DataCard not found: " << e.what() << std::endl;
     * }
     * @endcode
     */
    [[nodiscard]] const DataCard& get(const std::vector<double>& index) const {
        return get(FloatIndexVector(index));
    }

    /**
     * @brief Accesses a DataCard from the OPAT structure by index.
     * @param index The index vector of the DataCard to access.
     * @return A constant reference to the DataCard.
     * @throws std::out_of_range if the index is not found.
     */
    const DataCard& operator[](const FloatIndexVector& index) const;

    /**
     * @brief Accesses a DataCard from the OPAT structure by a standard vector of doubles.
     * This is a convenience overload that constructs a FloatIndexVector internally.
     * @param index The std::vector<double> representing the index of the DataCard to access.
     * @return A constant reference to the DataCard.
     * @throws std::out_of_range if the index is not found.
     * @example
     * @code
     * // Assuming 'opat_file' is an initialized opat::OPAT object
     * std::vector<double> my_index = {1.0, 2.5};
     * try {
     *     const opat::DataCard& card = opat_file[my_index];
     *     // Use the card
     * } catch (const std::out_of_range& e) {
     *     std::cerr << "DataCard not found: " << e.what() << std::endl;
     * }
     * @endcode
     */
    const DataCard& operator[](const std::vector<double>& index) const {
        return get(FloatIndexVector(index));
    }

    /**
     * @brief Calculates and returns the bounds (min and max values) for each dimension of the index vectors in the OPAT file.
     * @return A vector of Bounds structs, where each struct corresponds to a dimension of the index vectors.
     * The size of the returned vector will be equal to `header.numIndex`.
     * @example
     * @code
     * // Assuming 'opat_file' is an initialized opat::OPAT object
     * std::vector<opat::Bounds> all_bounds = opat_file.getBounds();
     * for (size_t i = 0; i < all_bounds.size(); ++i) {
     *     std::cout << "Dimension " << i << ": Min = " << all_bounds[i].min
     *               << ", Max = " << all_bounds[i].max << std::endl;
     * }
     * @endcode
     */
    [[nodiscard]] std::vector<Bounds> getBounds() const;
};

/**
 * @brief Reads an OPAT file and returns its contents as an OPAT structure.
 * 
 * This function validates the file's magic number, reads the header, card catalog, 
 * and all data cards, and constructs an OPAT object representing the file's contents.
 * 
 * @param filename Path to the OPAT file.
 * @return An OPAT structure containing the file's data.
 * @throws std::runtime_error if the file cannot be opened, is invalid, or has an incorrect magic number.
 * 
 * @code
 * OPAT file = opat::readOPAT("example.opat");
 * std::cout << file.header << std::endl;
 * @endcode
 */
OPAT readOPAT(const std::string& filename);

/**
 * @brief Reads the header of an OPAT file.
 * 
 * This function reads the header structure from the file and performs byte-swapping 
 * if the system is big-endian.
 * 
 * @param file Input file stream positioned at the start of the header.
 * @return A Header structure containing the file's metadata.
 * @throws std::runtime_error if the header cannot be read or is incomplete.
 * 
 * @code
 * std::ifstream file("example.opat", std::ios::binary);
 * opat::Header header = opat::readHeader(file);
 * header.print();
 * @endcode
 */
Header readHeader(std::ifstream &file);

/**
 * @brief Reads a CardCatalogEntry from the file.
 * 
 * This function reads a single entry from the card catalog, including its index, 
 * byte range, and SHA-256 hash. It also performs byte-swapping for compatibility 
 * with big-endian systems.
 * 
 * @param file Input file stream.
 * @param offset Offset to the entry in the file.
 * @param numIndex Number of indices in the entry.
 * @param hashPrecision Precision of the hash used for validation.
 * @return A CardCatalogEntry structure.
 * @throws std::runtime_error if the entry cannot be read or is incomplete.
 * 
 * @code
 * std::ifstream file("example.opat", std::ios::binary);
 * opat::CardCatalogEntry entry = opat::readCardCatalogEntry(file, 128, 3, 8);
 * std::cout << entry << std::endl;
 * @endcode
 */
CardCatalogEntry readCardCatalogEntry(std::ifstream &file, uint64_t offset, uint16_t numIndex, uint8_t hashPrecision);

/**
 * @brief Reads the CardCatalog from the file.
 * 
 * This function reads all entries in the card catalog, using the header information 
 * to determine the number of entries and their locations.
 * 
 * @param file Input file stream.
 * @param header The header of the OPAT file.
 * @return A CardCatalog structure.
 * @throws std::runtime_error if the card catalog cannot be read or is incomplete.
 * 
 * @code
 * std::ifstream file("example.opat", std::ios::binary);
 * opat::Header header = opat::readHeader(file);
 * opat::CardCatalog catalog = opat::readCardCatalog(file, header);
 * std::cout << catalog << std::endl;
 * @endcode
 */
CardCatalog readCardCatalog(std::ifstream &file, const Header &header);

/**
 * @brief Reads all DataCards from the file.
 * 
 * This function reads all DataCards in the OPAT file, using the card catalog to locate 
 * each card and its associated data.
 * 
 * @param file Input file stream.
 * @param header The header of the OPAT file.
 * @param cardCatalog The CardCatalog of the OPAT file.
 * @return A map of index vectors to DataCards.
 * @throws std::runtime_error if any DataCard cannot be read or is incomplete.
 * 
 * @code
 * std::ifstream file("example.opat", std::ios::binary);
 * opat::Header header = opat::readHeader(file);
 * opat::CardCatalog catalog = opat::readCardCatalog(file, header);
 * auto cards = opat::readDataCards(file, header, catalog);
 * @endcode
 */
std::unordered_map<FloatIndexVector, DataCard> readDataCards(std::ifstream &file, const Header &header, const CardCatalog &cardCatalog);

/**
 * @brief Reads a single DataCard from the file.
 * 
 * This function reads the header, table index, and table data for a single DataCard.
 * 
 * @param file Input file stream.
 * @param entry The CardCatalogEntry for the DataCard.
 * @return A DataCard structure.
 * @throws std::runtime_error if the DataCard cannot be read or is incomplete.
 * 
 * @code
 * std::ifstream file("example.opat", std::ios::binary);
 * opat::CardCatalogEntry entry = ...; // Retrieved from the catalog
 * opat::DataCard card = opat::readDataCard(file, entry);
 * @endcode
 */
DataCard readDataCard(std::ifstream &file, const CardCatalogEntry &entry);

/**
 * @brief Reads the header of a DataCard from the file.
 * 
 * This function reads the header structure of a DataCard and performs byte-swapping 
 * for compatibility with big-endian systems.
 * 
 * @param file Input file stream.
 * @param entry The CardCatalogEntry for the DataCard.
 * @return A CardHeader structure.
 * @throws std::runtime_error if the DataCard header cannot be read or is incomplete.
 * 
 * @code
 * std::ifstream file("example.opat", std::ios::binary);
 * opat::CardCatalogEntry entry = ...; // Retrieved from the catalog
 * opat::CardHeader header = opat::readDataCardHeader(file, entry);
 * @endcode
 */
CardHeader readDataCardHeader(std::ifstream &file, const CardCatalogEntry &entry);

/**
 * @brief Reads the TableIndex from a DataCard.
 * 
 * This function reads the table index of a DataCard, which maps table tags to their 
 * metadata and locations within the card.
 * 
 * @param file Input file stream.
 * @param entry The CardCatalogEntry for the DataCard.
 * @param header The CardHeader of the DataCard.
 * @return A TableIndex structure.
 * @throws std::runtime_error if the TableIndex cannot be read or is incomplete.
 * 
 * @code
 * std::ifstream file("example.opat", std::ios::binary);
 * opat::CardCatalogEntry entry = ...; // Retrieved from the catalog
 * opat::CardHeader header = opat::readDataCardHeader(file, entry);
 * opat::TableIndex index = opat::readTableIndex(file, entry, header);
 * @endcode
 */
TableIndex readTableIndex(std::ifstream &file, const CardCatalogEntry &entry, const CardHeader &header);

/**
 * @brief Reads an OPATTable from the file.
 * 
 * This function reads the row values, column values, and data for a single table 
 * within a DataCard.
 * 
 * @param file Input file stream.
 * @param cardEntry The CardCatalogEntry for the DataCard.
 * @param tableEntry The TableIndexEntry for the table.
 * @return An OPATTable structure.
 * @throws std::runtime_error if the table cannot be read or is incomplete.
 * 
 * @code
 * std::ifstream file("example.opat", std::ios::binary);
 * opat::CardCatalogEntry cardEntry = ...; // Retrieved from the catalog
 * opat::TableIndexEntry tableEntry = ...; // Retrieved from the table index
 * opat::OPATTable table = opat::readOPATTable(file, cardEntry, tableEntry);
 * @endcode
 */
OPATTable readOPATTable(std::ifstream &file, const CardCatalogEntry &cardEntry, const TableIndexEntry &tableEntry);

/**
 * @brief Checks if a file has the correct magic number for an OPAT file.
 * 
 * This function reads the first four bytes of the file and compares them to the 
 * expected "OPAT" magic number.
 * 
 * @param filename Path to the file.
 * @return True if the file has the correct magic number, false otherwise.
 * 
 * @code
 * bool isValid = opat::hasMagic("example.opat");
 * if (isValid) {
 *     std::cout << "Valid OPAT file!" << std::endl;
 * }
 * @endcode
 */
bool hasMagic(const std::string& filename);

/**
 * @brief Determines if the system is big-endian.
 * 
 * This utility function checks the system's endianness by examining the byte order 
 * of a test value.
 * 
 * @return True if the system is big-endian, false otherwise.
 * 
 * @code
 * if (opat::is_big_endian()) {
 *     std::cout << "System is big-endian." << std::endl;
 * }
 * @endcode
 */
bool is_big_endian();

/**
 * @brief Swaps the byte order of a value.
 * 
 * This function is useful for handling endianness when reading binary files. It 
 * ensures compatibility across systems with different byte orders.
 * 
 * @tparam T The type of the value (must be trivially copyable).
 * @param value The value to swap.
 * @return The value with its byte order reversed.
 * 
 * @code
 * uint16_t value = 0x1234;
 * uint16_t swapped = opat::swap_bytes(value);
 * @endcode
 */
template <typename T>
T swap_bytes(T value) {
    static_assert(std::is_trivially_copyable_v<T>, "swap_bytes only supports trivial types.");
    T result;
    auto src = reinterpret_cast<uint8_t*>(&value);
    auto* dest = reinterpret_cast<uint8_t*>(&result);
    for (size_t i = 0; i < sizeof(T); i++) {
        dest[i] = src[sizeof(T) - 1 - i];
    }
    return result;
}

} // namespace opat

#endif
