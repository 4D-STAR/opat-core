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
#include "opatIO.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <algorithm>
#include <iomanip>
#include <map>
#include <utility>
#include <cmath>
#include <limits>
#include <deque>
#include <cstdint>
#include "picosha2.h"

// Function to check system endianness
bool is_big_endian() {
    uint16_t test = 0x1;
    return reinterpret_cast<uint8_t*>(&test)[0] == 0;
}

// Generic function to swap bytes for any type
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

bool hasMagic(std::string filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    char magic[4];
    file.read(magic, 4);
    file.close();

    return std::string(magic, 4) == "OPAT";
}

// Constructor
OpatIO::OpatIO() {}

OpatIO::OpatIO(const std::string filename) : filename(filename) {
    if (!hasMagic(filename)) {
        throw std::runtime_error("The file provided is not an OPAT file (missing magic number!).");
    }
    load();
}

// Destructor
OpatIO::~OpatIO() {
    unload();
}

// Load the OPAT file
void OpatIO::load() {
    if (loaded) return;

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    readHeader(file);
    readTableIndex(file);

    loaded = true;
    file.close();
}

// // Unload the OPAT file
void OpatIO::unload() {
    if (!loaded) return;

    tableIndex.clear();
    while (!tableQueue.empty()) {
        tableQueue.pop_front();
    }

    loaded = false;
}

// Read the header from the file
void OpatIO::readHeader(std::ifstream &file) {
    file.read(reinterpret_cast<char*>(&header), sizeof(Header));
    if (file.gcount() != sizeof(Header)) {
        throw std::runtime_error("Error reading header from file");
    }

    if (is_big_endian()) {
        header.version = swap_bytes(header.version);
        header.numTables = swap_bytes(header.numTables);
        header.indexOffset = swap_bytes(header.indexOffset);
        header.numIndex = swap_bytes(header.numIndex);
    }
}

// Read the table index from the file
void OpatIO::readTableIndex(std::ifstream &file) {
    file.seekg(header.indexOffset, std::ios::beg);
    tableIndex.resize(header.numTables);
    long unsigned int indexReadBytes;
    for (uint32_t i = 0; i < header.numTables; i++) {
        indexReadBytes = 0;
        // Read the index vector in based on numIndex
        tableIndex.at(i).index.resize(header.numIndex);
        file.read(reinterpret_cast<char*>(tableIndex.at(i).index.data()), header.numIndex * sizeof(double));
        indexReadBytes += static_cast<int>(file.gcount());

        // Read the start and end position of the table in 
        file.read(reinterpret_cast<char*>(&tableIndex.at(i).byteStart), sizeof(uint64_t));
        indexReadBytes += static_cast<int>(file.gcount());
        file.read(reinterpret_cast<char*>(&tableIndex.at(i).byteEnd), sizeof(uint64_t));
        indexReadBytes += static_cast<int>(file.gcount());

        // Read the checksum in
        file.read(tableIndex.at(i).sha256, 32);
        indexReadBytes += static_cast<int>(file.gcount());

        // validate that the size of read data is correct
        if (indexReadBytes != header.numIndex * sizeof(double) + 32 + 2 * sizeof(uint64_t)) {
            throw std::runtime_error("Error reading table index from file");
        }
    }


    buildTableIDToIndex();
}

void OpatIO::buildTableIDToIndex(){
    tableIDToIndex.clear();
    int tableID = 0;
    std::vector<double> ind;
    ind.resize(header.numIndex);
    for (const auto &table : tableIndex) {
        ind.clear();
        for (const auto &index : table.index) {
            ind.push_back(index);
        }
        tableIDToIndex.emplace(tableID, ind);
        tableID++;
    }
    LookupEpsilon();
}

void OpatIO::LookupEpsilon() {
    /* 
     Get 10% of the minimum spacing between index values 
     in the tableIDToIndex map. This can be used
     to set the comparison distance when doing a reverse
     lookup (index -> tableID)
     */
    indexEpsilon.resize(header.numIndex);

    double epsilon;
    for (int i = 0; i < static_cast<int>(header.numIndex); i++) {
        epsilon = std::numeric_limits<double>::max();
        for (int j = 1; j < static_cast<int>(header.numTables); j++) {
            epsilon = std::min(epsilon, std::fabs(tableIDToIndex.at(j).at(i) - tableIDToIndex.at(j-1).at(i)));
        }
        if (epsilon < 1e-8) {
            indexEpsilon.at(i) = 1e-8;
        } else {
            indexEpsilon.at(i) = epsilon * 0.1;
        }
    }
}

int OpatIO::lookupTableID(std::vector<double> index) {
    std::vector<bool> IndexOkay;
    IndexOkay.resize(header.numIndex);
    int tableID = 0;
    for (const auto &tableMap : tableIDToIndex){
        // Loop through all index values and check if they are within epsilon for that index
        std::fill(IndexOkay.begin(), IndexOkay.end(), false);
        for (long unsigned int i = 0; i < index.size(); i++) {
            IndexOkay.at(i) = std::fabs(tableMap.second.at(i) - index.at(i)) < indexEpsilon.at(i);
        }
        // If all index values are within epsilon, return the table ID
        if (std::all_of(IndexOkay.begin(), IndexOkay.end(), [](bool i){return i;})) {
            return tableID;
        }
        tableID++;
    }
    // If no table is found, return -1 (sentinal value)
    return -1;
}

// Get a table from the queue
OPATTable OpatIO::getTableFromQueue(int tableID) {
    for (const auto &table : tableQueue) {
        if (table.first == tableID) {
            return table.second;
        }
    }
    throw std::out_of_range("Table not found!");
}

// Add a table to the queue
void OpatIO::addTableToQueue(int tableID, OPATTable table) {
    if (static_cast<int>(tableQueue.size()) >= maxQDepth) {
        removeTableFromQueue();
    }
    std::pair<int, OPATTable> IDTablePair = {tableID, table};
    tableQueue.push_back(IDTablePair);
}

// Remove a table from the queue
void OpatIO::removeTableFromQueue() {
    if (!tableQueue.empty()) {
        tableQueue.pop_front();
    }
}

// Flush the queue
void OpatIO::flushQueue() {
    while (!tableQueue.empty()) {
        tableQueue.pop_back();
        tableQueue.pop_front();
    }
}

// Get the OPAT version
uint16_t OpatIO::getOPATVersion() {
    return header.version;
}

// Get a table for given X and Z
OPATTable OpatIO::getTable(std::vector<double> index) {
    int tableID = lookupTableID(index);
    if (tableID == -1) {
        throw std::out_of_range("Index Not found!");
    }
    try {
        return getTableFromQueue(tableID);
    }
    catch(const std::out_of_range &e) { 
        return getTable(tableID);
    }
}

OPATTable OpatIO::getTable(int tableID) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    uint64_t byteStart = tableIndex[tableID].byteStart;

    file.seekg(byteStart, std::ios::beg);

    OPATTable table;

    // Step 1: Read N_R and N_T
    file.read(reinterpret_cast<char*>(&table.N_R), sizeof(uint32_t));
    file.read(reinterpret_cast<char*>(&table.N_T), sizeof(uint32_t));
    

    // Resize vectors to hold the correct number of elements
    table.logR.resize(table.N_R);
    table.logT.resize(table.N_T);
    table.logKappa.resize(table.N_R, std::vector<double>(table.N_T));

    // Step 2: Read logR values
    file.read(reinterpret_cast<char*>(table.logR.data()), table.N_R * sizeof(double));

    // Step 3: Read logT values
    file.read(reinterpret_cast<char*>(table.logT.data()), table.N_T * sizeof(double));

    // Step 4: Read logKappa values (flattened row-major order)
    for (size_t i = 0; i < table.N_R; ++i) {

        file.read(reinterpret_cast<char*>(table.logKappa[i].data()), table.N_T * sizeof(double));
    }

    if (!file) {
        throw std::runtime_error("Error reading table from file: " + filename);
    }

    addTableToQueue(tableID, table);
    file.close();
    return table;
}

// Set the maximum queue depth
void OpatIO::setMaxQDepth(int depth) {
    maxQDepth = depth;
}

int OpatIO::getMaxQDepth() {
    return maxQDepth;
}

// Set the filename
void OpatIO::setFilename(std::string filename) {
    if (loaded) {
        throw std::runtime_error("Cannot set filename while file is loaded");
    }
    this->filename = filename;
}

// Check if the file is loaded
bool OpatIO::isLoaded() {
    return loaded;
}

// Print the header
void OpatIO::printHeader() {
    std::cout << "Version: " << header.version << std::endl;
    std::cout << "Number of Tables: " << header.numTables << std::endl;
    std::cout << "Header Size: " << header.headerSize << std::endl;
    std::cout << "Index Offset: " << header.indexOffset << std::endl;
    std::cout << "Creation Date: " << header.creationDate << std::endl;
    std::cout << "Source Info: " << header.sourceInfo << std::endl;
    std::cout << "Comment: " << header.comment << std::endl;
    std::cout << "Number of Indices: " << header.numIndex << std::endl;
}

// Print the table index
void OpatIO::printTableIndex() {
    if (tableIndex.empty()) {
        std::cout << "No table indexes found." << std::endl;
        return;
    }

    // Print table header
    std::cout << std::left << std::setw(10);
    for (int i = 0; i < header.numIndex; i++) {
        std::cout << "Index " + std::to_string(i) << " " << std::setw(10);
    }   
    std::cout << std::setw(15) << "Byte Start"
        << std::setw(15) << "Byte End"
        << "Checksum (SHA-256)" << std::endl;

    std::cout << std::string(80, '=') << std::endl;  // Separator line

    // Print each entry in the table
    for (const auto &index : tableIndex) {
        std::cout << std::fixed << std::setprecision(4) << std::setw(10);
        for (int i = 0; i < header.numIndex; i++) {
            std::cout << index.index[i] << std::setw(10);
        }
        std::cout << std::setw(15) << index.byteStart
            << std::setw(15) << index.byteEnd
            << std::hex; // Switch to hex mode for checksum

        for (int i = 0; i < 8; ++i) {  // Print first 8 bytes of SHA-256 for brevity
            std::cout << std::setw(2) << std::setfill('0') << (int)index.sha256[i];
        }

        std::cout << "..." << std::dec << std::setfill(' ') << std::endl;  // Reset formatting
    }
}

void OpatIO::printTable(OPATTable table, uint32_t truncateDigits) {
    int printTo;
    bool truncate = false;
    if (table.N_R > truncateDigits) {
        printTo = truncateDigits;
        truncate = true;
    } else {
        printTo = table.N_R-1;
    }
    std::cout << "LogR (size: " << table.logR.size() << "): [";
    for (int i = 0; i < printTo; ++i) {
        std::cout << table.logR.at(i) << ", ";
    }
    if (truncate) {
        std::cout << "..., ";
        for (int i = truncateDigits; i > 1; --i) {
            std::cout << table.logR.at(table.logR.size() - i) << ", ";
        }
    }
    std::cout << table.logR.back() << "]" << std::endl;

    if (table.N_T > truncateDigits) {
        printTo = truncateDigits;
        truncate = true;
    } else {
        printTo = table.N_T-1;
    }
    std::cout << "LogT (size: " << table.logT.size() << "): [";
    for (int i = 0; i < printTo; ++i) {
        std::cout << table.logT.at(i) << ", ";
    }
    if (truncate) {
        std::cout << "..., ";
        for (int i = truncateDigits; i > 1; --i) {
            std::cout << table.logT.at(table.logT.size() - i) << ", ";
        }
    }
    std::cout << table.logT.back() << "]" << std::endl;

    bool truncateRow = false;
    bool truncateCol = false;
    int printToRow, printToCol;
    if (table.N_T > truncateDigits) {
        printToRow = truncateDigits;
        truncateRow = true;
    } else {
        printToRow = table.N_T-1;
    }
    if (table.N_R > truncateDigits) {
        printToCol = truncateDigits;
        truncateCol = true;
    } else {
        printToCol = table.N_R-1;
    }

    std::cout << "LogKappa (size: " << table.N_R << " x " << table.N_T << "): \n[";
    for (int rowIndex = 0; rowIndex < printToRow; rowIndex++) {
        std::cout << "[";
        for (int colIndex = 0; colIndex < printToCol; colIndex++) {
            std::cout << table.logKappa.at(rowIndex).at(colIndex) << ", ";
        }
        if (truncateRow) {
            std::cout << "..., ";
            for (int i = truncateDigits; i > 1; i--) {
                std::cout << table.logKappa.at(rowIndex).at(table.logKappa.at(rowIndex).size() - i) << ", ";
            }
        }
        std::cout << table.logKappa.at(rowIndex).back() << "],\n";
    } 
    if (truncateCol) {
        std::cout << ".\n.\n.\n";
        for (int rowIndex = truncateDigits; rowIndex > 1; rowIndex--) {
            std::cout << "[";
            for (int colIndex = 0; colIndex < printToCol; colIndex++) {
                std::cout << table.logKappa.at(rowIndex).at(colIndex) << ", ";
            }
            if (truncateRow) {
                std::cout << "..., ";
                for (int i = truncateDigits; i > 1; i--) {
                    std::cout << table.logKappa.at(rowIndex).at(table.logKappa.at(rowIndex).size() - i) << ", ";
                }
            }
            std::cout << table.logKappa.at(rowIndex).back() << "],\n";
        } 
        std::cout << "[";
        for (int colIndex = 0; colIndex < printToCol; colIndex++) {
            std::cout << table.logKappa.back().at(colIndex) << ", ";
        }
        if (truncateRow) {
            std::cout << "..., ";
            for (int i = truncateDigits; i > 1; i--) {
                std::cout << table.logKappa.back().at(table.logKappa.back().size() - i) << ", ";
            }
        }
        std::cout << table.logKappa.back().back() << "]";
    }
    std::cout << "]" << std::endl;
}

void OpatIO::printTable(std::vector<double> index, uint32_t truncateDigits) {
    int tableID = lookupTableID(index);
    OPATTable table = getTable(tableID);
    printTable(table, truncateDigits);
}

// Get the table index
std::vector<TableIndex> OpatIO::getTableIndex() {
    return tableIndex;
}

// Get the header
Header OpatIO::getHeader() {
    return header;
}

// Get the size of the index vector used
uint16_t OpatIO::getNumIndex() {
    return header.numIndex;
}

TableIndex OpatIO::getTableIndex(std::vector<double> index) {
    int tableID = lookupTableID(index);
    return tableIndex.at(tableID);
}

std::vector<unsigned char> OpatIO::computeChecksum(int tableID) {
    OPATTable table = getTable(tableID);
    std::vector<std::vector<double>> logKappa = table.logKappa;
    std::vector<double> flatData(logKappa.size() * logKappa.size());
    size_t offset = 0;
    for (const auto& row : logKappa) {
        for (const auto& val : row) {
            flatData[offset++] = val;
        }
    }
    std::vector<unsigned char> flatDataBytes(flatData.size() * sizeof(double));
    std::memcpy(flatDataBytes.data(), flatData.data(), flatDataBytes.size());
    std::vector<unsigned char> hash(picosha2::k_digest_size);
    picosha2::hash256(flatDataBytes.begin(), flatDataBytes.end(), hash.begin(), hash.end());
    return hash;
}

std::vector<unsigned char> OpatIO::computeChecksum(std::vector<double> index) {
    int tableID = lookupTableID(index);
    return computeChecksum(tableID);
}

bool OpatIO::validateAll() {
    for (const auto &table : tableIDToIndex) {
        std::vector<unsigned char> hash = computeChecksum(table.first);
        std::vector<unsigned char> storedHash(tableIndex.at(table.first).sha256, tableIndex.at(table.first).sha256 + 32);
        if (hash != storedHash) {
            return false;
        }
    }
    return true;
}