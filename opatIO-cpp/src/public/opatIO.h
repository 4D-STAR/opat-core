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
 * This file defines the core data structures and functions used for handling OPAT files. 
 * The OPAT format is designed for efficient storage and retrieval of multi-dimensional data tables, 
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
 *     if (opat::hasMagic(filename)) {
 *         opat::OPAT file = opat::readOPAT(filename);
 *         const auto& table = file.get({1.0, 2.0, 3.0}).get("table_tag");
 *         table.print();
 *     }
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
#include <deque>
#include <map>
#include <utility>
#include <cstdint>
#include <unordered_map>

#include "indexVector.h"

namespace opat {

/**
 * @brief Structure to hold the header information of an OPAT file.
 *
 * The header contains metadata about the file, such as its version, creation date, and offsets to key sections.
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
    uint8_t hashPrescision;  ///< Precision of the hash used for table validation.
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

/**
 * @brief Structure to hold the index information of a table within a DataCard.
 *
 * Each TableIndexEntry contains metadata about a table, such as its dimensions and location.
 */
struct TableIndexEntry {
    char tag[4];             ///< Tag identifying the table.
    uint64_t byteStart;      ///< Byte start position of the table in the card.
    uint64_t byteEnd;        ///< Byte end position of the table in the card.
    uint16_t numColumns;     ///< Number of columns in the table.
    uint16_t numRows;        ///< Number of rows in the table.
    char columnName[8];      ///< Name of the columns (optional).
    char rowName[8];         ///< Name of the rows (optional).
    char reserved[20];       ///< Reserved for future use.

    /**
     * @brief Stream insertion operator for printing the table index entry.
     * @param os Output stream.
     * @param entry TableIndexEntry to print.
     * @return Reference to the output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const TableIndexEntry& entry);
};

/**
 * @brief Structure to hold the index of tables within a DataCard.
 *
 * The TableIndex maps table tags to their corresponding TableIndexEntry, enabling fast lookups.
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
    const TableIndexEntry& get(const std::string& tag) const;

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

    uint32_t N_R; ///< Number of rows in the table.
    uint32_t N_C; ///< Number of columns in the table.

    /**
     * @brief Returns the size of the table as a pair of rows and columns.
     * @return A pair containing the number of rows and columns.
     */
    std::pair<double, double> size() const { return std::make_pair(N_R, N_C); }

    friend std::ostream& operator<<(std::ostream& os, const OPATTable& table);

    /**
     * @brief Accesses a table value by row and column.
     * @param row The row index.
     * @param column The column index.
     * @return A constant reference to the value at the specified row and column.
     * @throws std::out_of_range if the row or column index is out of bounds.
     */
    const double& operator()(uint32_t row, uint32_t column) const;

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
     * @return A constant reference to the value at the specified row and column.
     * @throws std::out_of_range if the row or column index is out of bounds.
     */
    const double& getData(uint32_t row, uint32_t column) const;

    /**
     * @brief Extracts a single row from the table.
     * @param row The row index to extract.
     * @return An OPATTable containing the specified row.
     * @throws std::out_of_range if the row index is out of bounds.
     */
    OPATTable getRow(uint32_t row) const;

    /**
     * @brief Extracts a single column from the table.
     * @param column The column index to extract.
     * @return An OPATTable containing the specified column.
     * @throws std::out_of_range if the column index is out of bounds.
     */
    OPATTable getColumn(uint32_t column) const;

    /**
     * @brief Retrieves all row values of the table.
     * @return An OPATTable containing the row values.
     */
    OPATTable getRowValues() const;

    /**
     * @brief Retrieves all column values of the table.
     * @return An OPATTable containing the column values.
     */
    OPATTable getColumnValues() const;

    /**
     * @brief Retrieves the raw data of the table.
     * @return A pointer to the raw data array.
     */
    const double* getRawData() const;

    /**
     * @brief Extracts a slice of the table.
     * @param rowSlice The range of rows to extract.
     * @param colSlice The range of columns to extract.
     * @return An OPATTable containing the specified slice.
     * @throws std::out_of_range if the slice indices are out of bounds.
     */
    OPATTable slice(const Slice& rowSlice, const Slice& colSlice) const;

    /**
     * @brief Converts the table to an ASCII representation.
     * @return A string containing the ASCII representation of the table.
     */
    std::string ascii() const;

    /**
     * @brief Prints the table to the standard output.
     */
    void print() const;
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
    const OPATTable& get(const std::string& tag) const;

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
    const OPATTable& operator[](const std::string_view tag) const;
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
    const DataCard& get(const FloatIndexVector& index) const;

    const DataCard& get(const std::vector<double>& index) const {
        return get(FloatIndexVector(index));
    }

    /**
     * @brief Accesses a DataCard from the OPAT structure by index.
     * @param index The index vector of the DataCard to access.
     * @return A constant reference to the DataCard.
     * @throws std::out_of_range if the index is not found.
     */
    const DataCard& operator[](const FloatIndexVector& index) const;

    const DataCard& operator[](const std::vector<double>& index) const {
        return get(FloatIndexVector(index));
    }
};

/**
 * @brief Reads an OPAT file and returns its contents as an OPAT structure.
 * @param filename Path to the OPAT file.
 * @return An OPAT structure containing the file's data.
 * @throws std::runtime_error if the file cannot be read or is invalid.
 */
OPAT readOPAT(std::string filename);

/**
 * @brief Reads the header of an OPAT file.
 * @param file Input file stream positioned at the start of the header.
 * @return A Header structure containing the file's metadata.
 */
Header readHeader(std::ifstream &file);

/**
 * @brief Reads a CardCatalogEntry from the file.
 * @param file Input file stream.
 * @param offset Offset to the entry in the file.
 * @param numIndex Number of indices in the entry.
 * @param hashPrecision Precision of the hash used for validation.
 * @return A CardCatalogEntry structure.
 */
CardCatalogEntry readCardCatalogEntry(std::ifstream &file, uint64_t offset, uint16_t numIndex, uint8_t hashPrecision);

/**
 * @brief Reads the CardCatalog from the file.
 * @param file Input file stream.
 * @param header The header of the OPAT file.
 * @return A CardCatalog structure.
 */
CardCatalog readCardCatalog(std::ifstream &file, const Header &header);

/**
 * @brief Reads all DataCards from the file.
 * @param file Input file stream.
 * @param header The header of the OPAT file.
 * @param cardCatalog The CardCatalog of the OPAT file.
 * @return A map of index vectors to DataCards.
 */
std::unordered_map<FloatIndexVector, DataCard> readDataCards(std::ifstream &file, const Header &header, const CardCatalog &cardCatalog);

/**
 * @brief Reads a single DataCard from the file.
 * @param file Input file stream.
 * @param entry The CardCatalogEntry for the DataCard.
 * @return A DataCard structure.
 */
DataCard readDataCard(std::ifstream &file, const CardCatalogEntry &entry);

/**
 * @brief Reads the header of a DataCard from the file.
 * @param file Input file stream.
 * @param entry The CardCatalogEntry for the DataCard.
 * @return A CardHeader structure.
 */
CardHeader readDataCardHeader(std::ifstream &file, const CardCatalogEntry &entry);

/**
 * @brief Reads the TableIndex from a DataCard.
 * @param file Input file stream.
 * @param entry The CardCatalogEntry for the DataCard.
 * @param header The CardHeader of the DataCard.
 * @return A TableIndex structure.
 */
TableIndex readTableIndex(std::ifstream &file, const CardCatalogEntry &entry, const CardHeader &header);

/**
 * @brief Reads an OPATTable from the file.
 * @param file Input file stream.
 * @param cardEntry The CardCatalogEntry for the DataCard.
 * @param tableEntry The TableIndexEntry for the table.
 * @return An OPATTable structure.
 */
OPATTable readOPATTable(std::ifstream &file, const CardCatalogEntry &cardEntry, const TableIndexEntry &tableEntry);

/**
 * @brief Checks if a file has the correct magic number for an OPAT file.
 * @param filename Path to the file.
 * @return True if the file has the correct magic number, false otherwise.
 */
bool hasMagic(std::string filename);

/**
 * @brief Determines if the system is big-endian.
 * @return True if the system is big-endian, false otherwise.
 */
bool is_big_endian();

/**
 * @brief Swaps the byte order of a value.
 * 
 * This function is useful for handling endianness when reading binary files.
 * It ensures compatibility across systems with different byte orders.
 * 
 * @tparam T The type of the value (must be trivially copyable).
 * @param value The value to swap.
 * @return The value with its byte order reversed.
 */
template <typename T>
T swap_bytes(T value) {
    static_assert(std::is_trivially_copyable<T>::value, "swap_bytes only supports trivial types.");
    T result;
    uint8_t* src = reinterpret_cast<uint8_t*>(&value);
    uint8_t* dest = reinterpret_cast<uint8_t*>(&result);
    for (size_t i = 0; i < sizeof(T); i++) {
        dest[i] = src[sizeof(T) - 1 - i];
    }
    return result;
}

} // namespace opat

#endif