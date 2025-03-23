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
#ifndef OPATIO_H
#define OPATIO_H

#include <fstream>
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <utility>
#include <cstdint>

/**
 * @brief Structure to hold the header information of an OPAT file.
 */
#pragma pack(1)
struct Header {
    char magic[4];           ///< Magic number to identify the file type
    uint16_t version;        ///< Version of the OPAT file format
    uint32_t numTables;      ///< Number of tables in the file
    uint32_t headerSize;     ///< Size of the header
    uint64_t indexOffset;    ///< Offset to the index section
    char creationDate[16];   ///< Creation date of the file
    char sourceInfo[64];     ///< Source information
    char comment[128];       ///< Comment section
    uint16_t numIndex;       ///< Size of index vector per table
    char reserved[24];       ///< Reserved for future use
};
#pragma pack()

/**
 * @brief Structure to hold the index information of a table in an OPAT file.
 */
struct TableIndex {
    std::vector<double> index; ///< Index vector for associated table
    uint64_t byteStart;      ///< Byte start position of the table
    uint64_t byteEnd;        ///< Byte end position of the table
    char sha256[32];         ///< SHA-256 hash of the table data
};

/**
 * @brief Structure to hold the data of an OPAT table.
 */
struct OPATTable {
    uint32_t N_R;                    ///< Number of R values
    uint32_t N_T;                    ///< Number of T values
    std::vector<double> logR;        ///< Logarithm of R values
    std::vector<double> logT;        ///< Logarithm of T values
    std::vector<std::vector<double>> logKappa; ///< Logarithm of Kappa values
};

/**
 * @brief Check if a file has the magic number of an OPAT file.
 * @param filename The name of the file.
 * @return True if the file has the magic number, false otherwise.
 */
bool hasMagic(std::string filename);

class opatIOTest; // Friend for gtest

/**
 * @brief Class to manage the input/output operations for OPAT files.
 */
class OpatIO {
private:
    Header header; ///< Header information of the OPAT file
    std::vector<TableIndex> tableIndex; ///< Index information of the tables
    std::deque<std::pair<int, OPATTable>> tableQueue; ///< Queue to manage table caching
    std::map<int, std::vector<double>> tableIDToIndex; ///< Map to store table ID to indexing
    std::vector<double> indexEpsilon; ///< Epsilon values for each index
    int maxQDepth = 20; ///< Maximum depth of the table queue
    std::string filename; ///< Filename of the OPAT file
    bool loaded = false; ///< Flag to indicate if the file is loaded

    /**
     * @brief Read the header from the file.
     * @param file The input file stream.
     */
    void readHeader(std::ifstream &file);

    /**
     * @brief Read the table index from the file.
     * @param file The input file stream.
     */
    void readTableIndex(std::ifstream &file);

    /**
     * @brief Get a table from the queue.
     * @param tableID The ID of the table.
     * @return The OPAT table.
     */
    OPATTable getTableFromQueue(int tableID);

    /**
     * @brief Add a table to the queue.
     * @param tableID The ID of the table.
     * @param table The OPAT table.
     */
    void addTableToQueue(int tableID, OPATTable table);

    /**
     * @brief Remove a table from the queue.
     */
    void removeTableFromQueue();

    /**
     * @brief Flush the table queue.
     */
    void flushQueue();

    /**
     * @brief Get a table by its ID.
     * @param tableID The ID of the table.
     * @return The OPAT table.
     */
    OPATTable getTable(int tableID);

    /**
     * @brief Print a table.
     * @param table The OPAT table.
     * @param truncateDigits Number of digits to truncate.
     */
    void printTable(OPATTable table, uint32_t truncateDigits=5);

    /**
     * @brief Lookup epsilon values for Index.
     */
    void LookupEpsilon();

    /**
     * @brief Build the table ID to Index mapping.
     */
    void buildTableIDToIndex();

public:
    /**
     * @brief Default constructor.
     */
    OpatIO();

    /**
     * @brief Constructor with filename.
     * @param filename The name of the OPAT file.
     */
    OpatIO(std::string filename);

    /**
     * @brief Destructor.
     */
    ~OpatIO();

    /**
     * @brief Get a table by index vector
     * @param index The index vector associated with the table to retrieve.
     * @throw std::out_of_range if the index is not found.
     * @return The OPAT table.
     */
    OPATTable getTable(std::vector<double> index);

    /**
     * @brief Set the maximum depth of the table queue.
     * @param depth The maximum depth.
     */
    void setMaxQDepth(int depth);

    /**
     * @brief Get the maximum depth of the table queue.
     * @return The maximum depth.
     */
    int getMaxQDepth();

    /**
     * @brief Set the filename of the OPAT file.
     * @param filename The name of the file.
     */
    void setFilename(std::string filename);

    /**
     * @brief Load the OPAT file.
     */
    void load();

    /**
     * @brief Unload the OPAT file.
     */
    void unload();

    /**
     * @brief Check if the file is loaded.
     * @return True if the file is loaded, false otherwise.
     */
    bool isLoaded();

    /**
     * @brief Print the header information.
     */
    void printHeader();

    /**
     * @brief Print the table index.
     */
    void printTableIndex();

    /**
     * @brief Print a table by X and Z values.
     * @param index The index vector associated with the table to print.
     * @param truncateDigits Number of digits to truncate.
     */
    void printTable(std::vector<double> index, uint32_t truncateDigits=5);

    /**
     * @brief Get the table index.
     * @return A vector of TableIndex structures.
     */
    std::vector<TableIndex> getTableIndex();

    /**
     * @brief Get the header information.
     * @return The Header structure.
     */
    Header getHeader();

    /**
     * @brief Lookup the table ID by X and Z values.
     * @param index The index vector associated with the table to lookup.
     * @return The table ID if index is found, otherwise -1.
     */
    int lookupTableID(std::vector<double> index);

    /**
     * @brief Lookup the closest table ID by X and Z values.
     * @param index The index vector associated with the table to lookup.
     * @return The closest table ID.
     */
    int lookupClosestTableID(std::vector<double> index);

    /**
     * @brief Get the version of the OPAT file format.
     * @return The version of the OPAT file format.
     */
    uint16_t getOPATVersion();

    /**
     * @brief Get size of the index vector per table in the OPAT file.
     * @return The size of the index vector per table.
     */
    uint16_t getNumIndex();

    /**
     * @brief Get the index vector for a given table ID.
     * @param index The index vector associated with the table to retrieve.
     * @return The full TableIndex entry for the table
     */
    TableIndex getTableIndex(std::vector<double> index);

    /**
     * @brief Get the checksum (sha256) for a given table ID.
     * @param tableID The ID of the table.
     * @return The checksum vector for the table.
     */
    std::vector<unsigned char> computeChecksum(int tableID);

    /**
     * @brief Get the checksum (sha256) for a given index vector.
     * @param index The index vector associated with the table to retrieve.
     * @return The checksum vector for the table.
     */
    std::vector<unsigned char> computeChecksum(std::vector<double> index);

    /**
     * @brief Validate the checksum of all tables.
     * @return True if all checksum are valid, false otherwise.
     */
    bool validateAll();
};

#endif