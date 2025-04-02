#include "opatIO.h"

#include <iostream>

int main() {
    std::string filename = "gs98hz.opat";
    opat::OPAT file = opat::readOPAT(filename);
    const auto& table = file.get({0.95, 0.001}).get("data");
    // you could print this with table.print(), however, we will show slicing
    opat::Slice rowSlice(0, 6); // Gets rows 0 to 5
    opat::Slice colSlice(25, 36); // Gets columns 25 to 35
    auto slicedTable = table.slice(rowSlice, colSlice);
    std::cout << "Sliced Table:\n" << slicedTable.ascii() << std::endl;

    // Print the header information
    std::cout << "Header Information:\n";
    std::cout << file.header << std::endl;

    // Print the card catalog information
    std::cout << "Card Catalog Information:\n";
    std::cout << file.cardCatalog << std::endl;

    // Get a specific DataCard
    const auto& card = file.get({0.95, 0.001});

    std::cout << "DataCard Information:\n";
    std::cout << card << std::endl;

    // Get a specific table from the DataCard
    // You could also use card["data"] or card.get("data")
    // to access the table directly
    const auto& dataTable = card.get("data");
    std::cout << "Data Table Information:\n";
    std::cout << dataTable << std::endl;

    // We can get out a specific row
    auto row = dataTable.getRow(5);
    std::cout << "Row 5 Information:\n";
    std::cout << row << std::endl;

    // We can get out a specific column
    auto column = dataTable.getColumn(5);
    std::cout << "Column 5 Information:\n";
    std::cout << column << std::endl;

    // We can get the row values (the values which parameterize the rows)
    auto rowValues = dataTable.getRowValues();
    std::cout << "Row Values Information:\n";
    std::cout << rowValues.ascii() << std::endl;

    // We can get the column values (the values which parameterize the columns)
    auto columnValues = dataTable.getColumnValues();
    std::cout << "Column Values Information:\n";
    std::cout << columnValues.ascii() << std::endl;

    // We can get the row parameter name
    std::cout << "Row Parameter Name: " << card.tableIndex.get("data").rowName << std::endl;

    // We can get the column parameter name
    std::cout << "Column Parameter Name: " << card.tableIndex.get("data").columnName << std::endl;
    return 0;
}