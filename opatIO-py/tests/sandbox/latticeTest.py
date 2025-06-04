from opatio import OPAT
from opatio import read_opat

from opatio.lattice.tableLattice import TableLattice

from opatio.index import FloatVectorIndex

opat = read_opat("gs98hz.opat")
lattice = TableLattice(opat)

query1 = FloatVectorIndex((0.7, 0.0564), opat.header.hashPrecision)
query2 = FloatVectorIndex((0.6, 0.0564), opat.header.hashPrecision)
query3 = FloatVectorIndex((0.65, 0.0564), opat.header.hashPrecision)
card1 = lattice.get(query1)
card2 = lattice.get(query2)
card3 = lattice.get(query3)

print(card1['data'].data[50, 18])
print(card3['data'].data[50, 18])
print(card2['data'].data[50, 18])
