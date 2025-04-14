from opatio import OPAT
from opatio.card.datacard import OPATTable
from opatio.card.datacard import OPATCell
import numpy as np

np.random.seed(42)

opat = OPAT()
opat.set_version(1)
opat.set_source("test_source")
opat.set_comment("test_comment")
opat.set_numIndex(2)

# Generate random data for the card
num_rows = 10
num_cols = 5

num_tables = 10

cards = list()
for tableID in range(num_tables): 
    data = np.random.rand(num_rows, num_cols, 4).astype(np.float64)

    column_values = np.random.rand(num_cols).astype(np.float64)
    column_name = "TC {}".format(tableID)

    row_values = np.random.rand(num_rows).astype(np.float64)
    row_name = "TR {}".format(tableID)

    index_vector = np.random.rand(2).astype(np.float64)

    card = opat.add_table(index_vector, "data", column_values, row_values, data, columnName=column_name, rowName=row_name)
    cards.append((index_vector, card))

for iv, card in cards:
    squareTable = card["data"].data**2
    newTable = OPATTable(columnValues=card["data"].columnValues, rowValues=card["data"].rowValues, data=squareTable)
    card.add_table("square", newTable, columnName="TC2", rowName="TR2")
    opat.add_card(iv, card)
    break
# Save the OPAT file
opat.save_as_ascii("test.dat")
opat.save("test.opat")

# Load the OPAT file
from opatio import read_opat
opat2 = read_opat("test.opat")
opat2.save_as_ascii("test2.dat")

# try conversion
from opatio.convert import OPALI_2_OPAT
OPALI_2_OPAT("GS98hz", "gs98hz.opat", saveAsASCII=True)

# Load the OPAT file
opat3 = read_opat("gs98hz.opat")
opat3.save_as_ascii("gs98hz2.dat")




