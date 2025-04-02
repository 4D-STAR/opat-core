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
#include "indexVector.h"

#include <fstream>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <algorithm>
#include <sys/types.h>
#include <unordered_map>
#include <cmath>
#include <cstdint>
#include <memory>
#include "picosha2.h"

namespace opat {
    // Function to check system endianness
    // Returns true if the system is big-endian, false otherwise
    bool is_big_endian() {
        uint16_t test = 0x1;
        return reinterpret_cast<uint8_t*>(&test)[0] == 0;
    }

    // Checks if the file has the "OPAT" magic number at the beginning
    bool hasMagic(std::string filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            return false; // File could not be opened
        }

        char magic[4];
        file.read(magic, 4); // Read the first 4 bytes
        file.close();

        return std::string(magic, 4) == "OPAT"; // Check if it matches "OPAT"
    }

    // Reads an OPAT file and constructs an OPAT object
    OPAT readOPAT(std::string filename) {
        // Verify the file has the correct magic number
        bool isOPAT = hasMagic(filename);
        if (!isOPAT) {
            throw std::runtime_error("File is not a valid OPAT file: " + filename);
        }

        // Open the file in binary mode
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + filename);
        }

        // Read the header and validate the magic number
        Header header = readHeader(file);
        if (std::string(header.magic, 4) != "OPAT") {
            throw std::runtime_error("Invalid OPAT file: " + filename);
        }

        // Read the card catalog and data cards
        CardCatalog cardCatalog = readCardCatalog(file, header);
        std::unordered_map<FloatIndexVector, DataCard> cards = readDataCards(file, header, cardCatalog);

        // Construct and return the OPAT object
        OPAT opat;
        opat.header = header;
        opat.cardCatalog = cardCatalog;
        opat.cards = std::move(cards);

        return opat;
    }

    // Reads the header of the OPAT file
    Header readHeader(std::ifstream &file) {
        Header header;
        file.read(reinterpret_cast<char*>(&header), sizeof(Header)); // Read the header structure
        if (file.gcount() != sizeof(Header)) {
            throw std::runtime_error("Error reading header from file");
        }

        // Swap bytes if the system is big-endian
        if (is_big_endian()) {
            header.version = swap_bytes(header.version);
            header.numTables = swap_bytes(header.numTables);
            header.indexOffset = swap_bytes(header.indexOffset);
            header.numIndex = swap_bytes(header.numIndex);
        }
        return header;
    }

    // Reads a single entry from the card catalog
    CardCatalogEntry readCardCatalogEntry(std::ifstream &file, uint64_t offset, uint16_t numIndex, uint8_t hashPrecision) {
        CardCatalogEntry entry;
        std::vector<double> index(numIndex);

        file.seekg(offset, std::ios::beg); // Move to the specified offset
        file.read(reinterpret_cast<char*>(index.data()), numIndex * sizeof(double)); // Read index values
        file.read(reinterpret_cast<char*>(&entry.byteStart), sizeof(uint64_t)); // Read byte start
        file.read(reinterpret_cast<char*>(&entry.byteEnd), sizeof(uint64_t)); // Read byte end
        file.read(entry.sha256, 32); // Read SHA-256 hash

        FloatIndexVector indexVector(index, hashPrecision);
        entry.index = indexVector;

        // Swap bytes if the system is big-endian
        if (is_big_endian()) {
            entry.byteStart = swap_bytes(entry.byteStart);
            entry.byteEnd = swap_bytes(entry.byteEnd);
        }
        return entry;
    }

    // Reads the card catalog from the file
    CardCatalog readCardCatalog(std::ifstream &file, const Header &header) {
        CardCatalog cardCatalog;
        cardCatalog.tableIndex.reserve(header.numTables); // Reserve space for entries
        uint64_t entrySize = 48 + sizeof(double) * header.numIndex; // Calculate entry size

        for (uint32_t i = 0; i < header.numTables; i++) {
            uint64_t offset = header.indexOffset + (i * entrySize); // Calculate offset for each entry
            CardCatalogEntry entry = readCardCatalogEntry(file, offset, header.numIndex, header.hashPrescision);
            cardCatalog.tableIndex.emplace(entry.index, entry); // Add entry to the catalog
        }
        return cardCatalog;
    }

    // Reads all data cards from the file
    std::unordered_map<FloatIndexVector, DataCard> readDataCards(std::ifstream &file, const Header &header, const CardCatalog &cardCatalog) {
        std::unordered_map<FloatIndexVector, DataCard> cards;
        for (const auto &[indexVector, entry] : cardCatalog.tableIndex) {
            DataCard dataCard = readDataCard(file, entry); // Read each data card
            cards.emplace(indexVector, std::move(dataCard)); // Add to the map
        }
        return cards;
    }

    // Reads a single data card from the file
    DataCard readDataCard(std::ifstream &file, const CardCatalogEntry &entry) {
        CardHeader header = readDataCardHeader(file, entry); // Read the card header
        TableIndex tableIndex = readTableIndex(file, entry, header); // Read the table index

        std::unordered_map<std::string, OPATTable> tableData;
        for (const auto &[tag, tableEntry] : tableIndex.tableIndex) {
            OPATTable table = readOPATTable(file, entry, tableEntry); // Read each table
            tableData.emplace(tag, std::move(table)); // Add to the map
        }

        DataCard dataCard;
        dataCard.header = header;
        dataCard.tableIndex = tableIndex;
        dataCard.tableData = std::move(tableData);

        return dataCard;
    }

    // Reads the header of a data card
    CardHeader readDataCardHeader(std::ifstream &file, const CardCatalogEntry &entry) {
        CardHeader header;
        file.seekg(entry.byteStart, std::ios::beg);
        file.read(reinterpret_cast<char*>(&header), sizeof(CardHeader));
        if (file.gcount() != sizeof(CardHeader)) {
            throw std::runtime_error("Error reading data card header from file");
        }
        if (is_big_endian()) {
            header.numTables = swap_bytes(header.numTables);
            header.headerSize = swap_bytes(header.headerSize);
            header.indexOffset = swap_bytes(header.indexOffset);
            header.cardSize = swap_bytes(header.cardSize);
        }
        return header;
    }

    // Reads the table index of a data card
    TableIndex readTableIndex(std::ifstream &file, const CardCatalogEntry &entry, const CardHeader &header) {
        TableIndex tableIndex;
        file.seekg(entry.byteStart + header.indexOffset, std::ios::beg);
        for (uint32_t i = 0; i < header.numTables; i++) {
            TableIndexEntry indexEntry;
            file.read(reinterpret_cast<char*>(&indexEntry), sizeof(TableIndexEntry));
            if (file.gcount() != sizeof(TableIndexEntry)) {
                throw std::runtime_error("Error reading table index from file");
            }
            if (is_big_endian()) {
                indexEntry.numColumns = swap_bytes(indexEntry.numColumns);
                indexEntry.numRows = swap_bytes(indexEntry.numRows);
                indexEntry.byteStart = swap_bytes(indexEntry.byteStart);
                indexEntry.byteEnd = swap_bytes(indexEntry.byteEnd);
            }
            tableIndex.tableIndex[indexEntry.tag] = indexEntry;
        }
        return tableIndex;
    }

    // Reads an OPAT table from the file
    OPATTable readOPATTable(std::ifstream &file, const CardCatalogEntry &cardEntry, const TableIndexEntry &tableEntry) {
        std::unique_ptr<double[]> rowValues(new double[tableEntry.numRows]);
        std::unique_ptr<double[]> columnValues(new double[tableEntry.numColumns]);
        std::unique_ptr<double[]> data(new double[tableEntry.numRows * tableEntry.numColumns]);

        file.seekg(cardEntry.byteStart + tableEntry.byteStart, std::ios::beg);
        file.read(reinterpret_cast<char*>(rowValues.get()), tableEntry.numRows * sizeof(double));
        file.read(reinterpret_cast<char*>(columnValues.get()), tableEntry.numColumns * sizeof(double));
        file.read(reinterpret_cast<char*>(data.get()), tableEntry.numRows * tableEntry.numColumns * sizeof(double));

        if (file.gcount() != tableEntry.numRows * tableEntry.numColumns * sizeof(double)) {
            throw std::runtime_error("Error reading OPAT table from file");
        }

        OPATTable table;
        table.rowValues = std::move(rowValues);
        table.columnValues = std::move(columnValues);
        table.data = std::move(data);

        table.N_R = tableEntry.numRows;
        table.N_C = tableEntry.numColumns;
        return table;
    }

    void Header::print() const {
        std::cout << "Header:\n";
        std::cout << "  Magic: " << std::string(magic, 4) << "\n";
        std::cout << "  Version: " << version << "\n";
        std::cout << "  NumTables: " << numTables << "\n";
        std::cout << "  IndexOffset: " << indexOffset << "\n";
        std::cout << "  NumIndex: " << numIndex << "\n";
        std::cout << "  HashPrecision: " << static_cast<int>(hashPrescision) << "\n";
        std::cout << "  Comment: " << comment << "\n";
        std::cout << "  Source: " << sourceInfo << "\n";
        std::cout << "  Creation Date: " << creationDate << std::endl;
    }

    // Utility functions
    const DataCard& OPAT::get(const FloatIndexVector& index) const {
        auto it = cards.find(index);
        if (it != cards.end()) {
            return it->second;
        } else {
            throw std::runtime_error("Card not found for the given index.");
        }
    }

    const DataCard& OPAT::operator[](const FloatIndexVector& index) const {
        return get(index);
    }

    const OPATTable& DataCard::get(const std::string& tag) const {
        auto it = tableData.find(tag);
        if (it != tableData.end()) {
            return it->second;
        } else {
            throw std::runtime_error("Table not found for the given tag.");
        }
    }

    const OPATTable& DataCard::operator[](const std::string& tag) const{
        return get(tag);
    }
    const OPATTable& DataCard::operator[](const char* tag) const {
        return get(std::string(tag));
    }
    const OPATTable& DataCard::operator[](const std::string_view tag) const {
        return get(std::string(tag));
    }

    const TableIndexEntry& TableIndex::get(const std::string& tag) const {
        auto it = tableIndex.find(tag);
        if (it != tableIndex.end()) {
            return it->second;
        } else {
            throw std::out_of_range("Tag not found in TableIndex");
        }
    }
    const TableIndexEntry& TableIndex::operator[](const std::string& tag) const {
        return get(tag);
    }


    const double& OPATTable::operator()(uint32_t row, uint32_t column) const {
        return getData(row, column);
    }

    OPATTable OPATTable::operator()(const Slice& rowSlice, const Slice& colSlice) const {
        return slice(rowSlice, colSlice);
    }

    const double& OPATTable::getData(uint32_t row, uint32_t column) const {
        if (row >= N_R || column >= N_C) {
            throw std::out_of_range("Index out of range");
        }
        if (row < 0 || column < 0) {
            throw std::out_of_range("Index out of range");
        }
        if (data == nullptr) {
            throw std::runtime_error("Data not initialized");
        }
        return data[row * N_C + column];
    }

    OPATTable OPATTable::getRow(uint32_t row) const {
        OPATTable rowData;
        rowData.N_C = N_C;
        rowData.N_R = 1;
        rowData.data = std::make_unique<double[]>(N_C);
        rowData.rowValues = std::make_unique<double[]>(1);
        rowData.columnValues = std::make_unique<double[]>(N_C);
        rowData.rowValues[0] = rowValues[row];
        for (uint32_t i = 0; i < N_C; ++i) {
            rowData.data[i] = getData(row, i);
            rowData.columnValues[i] = columnValues[i];
        }
        return rowData;
    }

    OPATTable OPATTable::getColumn(uint32_t column) const {
        OPATTable columnData;
        columnData.N_R = N_R;
        columnData.N_C = 1;

        columnData.data = std::make_unique<double[]>(N_R);
        columnData.rowValues = std::make_unique<double[]>(N_R);
        columnData.columnValues = std::make_unique<double[]>(1);
        columnData.columnValues[0] = columnValues[column];
        for (uint32_t i = 0; i < N_R; ++i) {
            columnData.data[i] = getData(i, column);
            columnData.rowValues[i] = rowValues[i];
        }
        return columnData;
    }

    OPATTable OPATTable::getRowValues() const {
        if (rowValues == nullptr) {
            throw std::runtime_error("Row values not initialized");
        }
        if (N_R == 0) {
            throw std::runtime_error("Row values not initialized");
        }
        OPATTable rowValuesTable;
        rowValuesTable.N_R = N_R;
        rowValuesTable.N_C = 1;
        rowValuesTable.data = std::make_unique<double[]>(N_R);
        rowValuesTable.rowValues = std::make_unique<double[]>(N_R);
        rowValuesTable.columnValues = std::make_unique<double[]>(1);
        rowValuesTable.columnValues[0] = 0;
        for (uint32_t i = 0; i < N_R; ++i) {
            rowValuesTable.data[i] = rowValues[i];
            rowValuesTable.rowValues[i] = i;
        }
        return rowValuesTable;
    }

    OPATTable OPATTable::getColumnValues() const {
        if (columnValues == nullptr) {
            throw std::runtime_error("Column values not initialized");
        }
        if (N_C == 0) {
            throw std::runtime_error("Column values not initialized");
        }
        OPATTable columnValuesTable;
        columnValuesTable.N_R = 1;
        columnValuesTable.N_C = N_C;
        columnValuesTable.data = std::make_unique<double[]>(N_C);
        columnValuesTable.rowValues = std::make_unique<double[]>(1);
        columnValuesTable.columnValues = std::make_unique<double[]>(N_C);
        columnValuesTable.rowValues[0] = 0;
        for (uint32_t i = 0; i < N_C; ++i) {
            columnValuesTable.data[i] = columnValues[i];
            columnValuesTable.columnValues[i] = i;
        }
        return columnValuesTable;
    }

    const double* OPATTable::getRawData() const {
        if (data == nullptr) {
            throw std::runtime_error("Data not initialized");
        }
        if (N_R == 0 || N_C == 0) {
            throw std::runtime_error("Data not initialized");
        }
        return data.get();
    }

    OPATTable OPATTable::slice(const Slice& rowSlice, const Slice& colSlice) const {
        if (rowSlice.start >= N_R || rowSlice.end > N_R || colSlice.start >= N_C || colSlice.end > N_C) {
            throw std::out_of_range("Slice out of range");
        }
        if (rowSlice.start < 0 || rowSlice.end < 0 || colSlice.start < 0 || colSlice.end < 0) {
            throw std::out_of_range("Slice out of range");
        }
        OPATTable slicedTable;
        slicedTable.N_R = rowSlice.end - rowSlice.start;
        slicedTable.N_C = colSlice.end - colSlice.start;
        slicedTable.data = std::make_unique<double[]>(slicedTable.N_R * slicedTable.N_C);
        slicedTable.rowValues = std::make_unique<double[]>(slicedTable.N_R);
        slicedTable.columnValues = std::make_unique<double[]>(slicedTable.N_C);
        for (uint32_t i = rowSlice.start; i < rowSlice.end; ++i) {
            for (uint32_t j = colSlice.start; j < colSlice.end; ++j) {
                slicedTable.data[(i - rowSlice.start) * slicedTable.N_C + (j - colSlice.start)] = getData(i, j);
                slicedTable.rowValues[i - rowSlice.start] = rowValues[i];
                slicedTable.columnValues[j - colSlice.start] = columnValues[j];
            }
        }
        return slicedTable;
    }

    std::string OPATTable::ascii() const {
        std::string result;
        for (uint32_t i = 0; i < N_R; ++i) {
            for (uint32_t j = 0; j < N_C; ++j) {
                result += std::to_string(data[i * N_C + j]) + " ";
            }
            result += "\n";
        }
        return result;
    }

    void OPATTable::print() const {
        std::cout << ascii();
    }

    // Overloading << operator for printing
    std::ostream& operator<<(std::ostream& os, const Header& header) {
        os << "Header(Magic: " << std::string(header.magic, 4)
        << ", Version: " << header.version
        << ", NumTables: " << header.numTables
        << ", IndexOffset: " << header.indexOffset
        << ", NumIndex: " << header.numIndex
        << ", HashPrecision: " << static_cast<int>(header.hashPrescision) << ")";
        return os;
    }
    std::ostream& operator<<(std::ostream& os, const CardHeader& header) {
        os << "Magic: " << std::string(header.magic, 4) << "\n"
            << "Number of Tables: " << header.numTables << "\n"
            << "Header Size: " << header.headerSize << "\n"
            << "Index Offset: " << header.indexOffset << "\n"
            << "Card Size: " << header.cardSize << "\n"
            << "Comment: " << header.comment << "\n";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const CardCatalogEntry& entry) {
        os << "CardCatalogEntry(I=[";
        for (const auto& val : entry.index.getVector()) {
            os << val << " ";
        }
        os << "], Byte Start: " << entry.byteStart
            << ", Byte End: " << entry.byteEnd
            << ", SHA-256: ";
        for (int i = 0; i < 8; ++i) {  // Print first 8 bytes of SHA-256 for brevity
            os << std::hex << static_cast<int>(entry.sha256[i]);
        }
        os << "...)";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const CardCatalog& catalog) {
        os << "CardCatalog(" << catalog.tableIndex.size() << " entries)";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const TableIndexEntry& entry) {
        os << "TableIndexEntry(Tag: " << std::string(entry.tag, 4)
            << ", Byte Start: " << entry.byteStart
            << ", Byte End: " << entry.byteEnd
            << ", Num Columns: " << entry.numColumns
            << ", Num Rows: " << entry.numRows
            << ", Column Name: " << std::string(entry.columnName, 8)
            << ", Row Name: " << std::string(entry.rowName, 8) << ")";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const TableIndex& index) {
        for (const auto& entry : index.tableIndex) {
            os << entry.second << "\n";
        }
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const OPATTable& table) {
        os << "OPATTable(N_R: " << table.N_R << ", N_C: " << table.N_C << ")";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const DataCard& card) {
        os << "DataCard(Header: " << card.header << ", Table Index: " << card.tableIndex << ")";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const OPAT& opat) {
        os << "OPAT(Header: " << opat.header << ", Card Catalog: " << opat.cardCatalog << ")";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const Slice& slice) {
        os << "Slice(Start: " << slice.start << ", End: " << slice.end << ")";
        return os;
    }
} // namespace opat