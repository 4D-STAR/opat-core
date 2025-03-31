import struct
from typing import List
import numpy as np

from opatio.base.header import Header
from opatio import OPAT

from opatio.catalog.entry import CardCatalogEntry
from opatio.card.datacard import DataCard
from opatio.card.datacard import CardHeader
from opatio.card.datacard import OPATTable
from opatio.card.datacard import CardIndexEntry

def read_opat(filename: str) -> OPAT:
    """
    Load an OPAT file.

    Parameters
    ----------
    filename : str
        The name of the file to load.

    Returns
    -------
    OPAT
        The loaded OPAT object.

    Examples
    --------
    >>> opat = read_opat("example.opat")
    >>> print(opat.header)

    Notes
    -----
    This function reads the header, catalog, and data cards from the OPAT file.
    """
    opat = OPAT()
    with open(filename, 'rb') as f:
        headerBytes: bytes = f.read(256)
        unpackedHeader = struct.unpack("<4s H I I Q 16s 64s 128s H B 23s", headerBytes)
        loadedHeader = Header(
            magic = unpackedHeader[0].decode().replace("\x00", ""),
            version = unpackedHeader[1],
            numCards = unpackedHeader[2],
            headerSize = unpackedHeader[3],
            catalogOffset = unpackedHeader[4],
            creationDate = unpackedHeader[5].decode().replace("\x00", ""),
            sourceInfo = unpackedHeader[6].decode().replace("\x00", ""),
            comment = unpackedHeader[7].decode().replace("\x00", ""),
            numIndex = unpackedHeader[8],
            hashPrecision = unpackedHeader[9],
            reserved = unpackedHeader[10]
        )
        opat.header = loadedHeader
        f.seek(opat.header.catalogOffset)
        tableIndices: List[CardCatalogEntry] = []
        tableIndexChunkSize = 16 + loadedHeader.numIndex*8
        tableIndexFMTString = "<"+"d"*loadedHeader.numIndex+"QQ"
        while tableIndexEntryBytes := f.read(tableIndexChunkSize):
            unpackedTableIndexEntry = struct.unpack(tableIndexFMTString, tableIndexEntryBytes)
            checksum = f.read(32)
            index = unpackedTableIndexEntry[:loadedHeader.numIndex]
            tableIndexEntry = CardCatalogEntry(
                index = index,
                byteStart = unpackedTableIndexEntry[loadedHeader.numIndex],
                byteEnd = unpackedTableIndexEntry[loadedHeader.numIndex+1],
                sha256 = checksum
            )
            tableIndices.append(tableIndexEntry)
        
        # Read the card tables
        for entry in tableIndices:
            startByte = entry.byteStart
            endByte = entry.byteEnd
            f.seek(startByte)
            cardBytes = f.read(endByte - startByte)
            card = load_data_card(cardBytes)
            opat.add_card(entry.index, card)

    return opat

def load_data_card(b: bytes) -> DataCard:
    """
    Load a DataCard from bytes.

    Parameters
    ----------
    b : bytes
        The bytes to load into a DataCard.

    Returns
    -------
    DataCard
        The loaded DataCard object.

    Examples
    --------
    >>> with open("example.datacard", "rb") as f:
    ...     data = f.read()
    >>> card = loadDataCard(data)
    >>> print(card.header)

    Notes
    -----
    This function parses the header and tables from the provided bytes. This is
    not intended to be used by the end user; rather, this is a utility function
    used by load_opat.
    """
    newCard = DataCard()
    headerUnpacked = struct.unpack("<4s I I Q Q 128s 100s", b[:256])
    header = CardHeader(
        numTables = headerUnpacked[1],
        headerSize = headerUnpacked[2],
        indexOffset = headerUnpacked[3],
        cardSize = headerUnpacked[4],
        comment = headerUnpacked[5].decode().replace("\x00", ""),
        reserved= headerUnpacked[6]
    )
    newCard.header = header.copy()
    for indexEntry in range(header.numTables):
        startByte = header.indexOffset + indexEntry*64
        indexBytes = b[startByte:startByte+64]
        unpackedIndexEntry = struct.unpack("<8s Q Q H H 8s 8s 20s", indexBytes)

        tableTag = unpackedIndexEntry[0].decode().replace("\x00", "")
        tableByteStart = unpackedIndexEntry[1]
        tableByteEnd = unpackedIndexEntry[2]
        tableNumColumns = unpackedIndexEntry[3]
        tableNumRows = unpackedIndexEntry[4]
        tableColumnName = unpackedIndexEntry[5].decode().replace("\x00", "")
        tableRowName = unpackedIndexEntry[6].decode().replace("\x00", "")

        tableBytes = b[tableByteStart:tableByteEnd]
        rawData = struct.unpack(f"<{tableNumRows}d{tableNumColumns}d{tableNumRows*tableNumColumns}d", tableBytes)

        rowValues = np.array(rawData[:tableNumRows], dtype=np.float64)
        columnValues = np.array(rawData[tableNumRows:tableNumRows+tableNumColumns], dtype=np.float64)
        dataArray = np.array(rawData[tableNumRows+tableNumColumns:], dtype=np.float64).reshape((tableNumRows, tableNumColumns))

        newTable = OPATTable(rowValues=rowValues, columnValues=columnValues, data=dataArray)

        newCard.add_table(tableTag, newTable, columnName=tableColumnName, rowName=tableRowName)

    return newCard
