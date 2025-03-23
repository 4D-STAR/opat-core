import struct
import numpy as np
from datetime import datetime

from dataclasses import dataclass

from typing import Iterable, List, Tuple
from collections.abc import Iterable as collectionIterable

import hashlib

import os

@dataclass
class Header:
    """
    @brief Structure to hold the header information of an OPAT file.
    """
    magic: str           #< Magic number to identify the file type
    version: int         #< Version of the OPAT file format
    numTables: int       #< Number of tables in the file
    headerSize: int      #< Size of the header
    indexOffset: int     #< Offset to the index section
    creationDate: str    #< Creation date of the file
    sourceInfo: str      #< Source information
    comment: str         #< Comment section
    numIndex: int        #< Number of values to use when indexing table
    reserved: bytes      #< Reserved for future use

@dataclass
class TableIndex:
    """
    @brief Structure to hold the index information of a table in an OPAT file.
    """
    index: List[float]   #< Index values of the table
    byteStart: int       #< Byte start position of the table
    byteEnd: int         #< Byte end position of the table
    sha256: bytes        #< SHA-256 hash of the table data

@dataclass
class OPATTable:
    """
    @brief Structure to hold the data of an OPAT table.
    """
    N_R: int                     #< Number of R values
    N_T: int                     #< Number of T values
    logR: Iterable[float]        #< Logarithm of R values
    logT: Iterable[float]        #< Logarithm of T values
    logKappa: Iterable[Iterable[float]] #< Logarithm of Kappa values

def make_default_header() -> Header:
    """
    @brief Create a default header for an OPAT file.
    @return The default header.
    """
    return Header(
        magic="OPAT",
        version=1,
        numTables=0,
        headerSize=256,
        indexOffset=0,
        creationDate=datetime.now().strftime("%b %d, %Y"),
        sourceInfo="no source provided by user",
        comment="default header",
        numIndex=2,
        reserved=b"\x00" * 24
    )

class OpatIO:
    """
    @brief Class for handling OPAT file input/output operations.
    This class provides methods to validate, manipulate, and save OPAT files. It includes functionalities to validate character arrays, 1D arrays, and 2D arrays, compute checksums, set header information, add tables, and save the OPAT file in both ASCII and binary formats.
    Attributes:
        header (Header): The header of the OPAT file.
        tables (List[Tuple[Tuple[float, float], OPATTable]]): A list of tables in the OPAT file.
    Methods:
        validate_char_array_size(s: str, nmax: int) -> bool:
            Validate the size of a character array.
        validate_logKappa(logKappa):
            Validate the logKappa array.
        validate_1D(arr, name: str):
            Validate a 1D array.
        compute_checksum(data: bytes) -> bytes:
            Compute the SHA-256 checksum of the given data.
        set_version(version: int) -> int:
            Set the version of the OPAT file.
        set_source(source: str) -> str:
            Set the source information of the OPAT file.
        set_comment(comment: str) -> str:
            Set the comment of the OPAT file.
        add_table(X: float, Z: float, logR: Iterable[float], logT: Iterable[float], logKappa: Iterable[Iterable[float]]):
            Add a table to the OPAT file.
        _header_bytes() -> bytes:
            Convert the header to bytes.
        _table_bytes(table: OPATTable) -> Tuple[bytes, bytes]:
            Convert a table to bytes.
        _tableIndex_bytes(tableIndex: TableIndex) -> bytes:
            Convert a table index to bytes.
        __repr__() -> str:
            Get the string representation of the OpatIO object.
        _format_table_as_string(table: OPATTable, X: float, Z: float) -> str:
            Format a table as a string.
        print_table_indexes(table_indexes: List[TableIndex]) -> str:
            Print the table indexes.
        save_as_ascii(filename: str) -> str:
            Save the OPAT file as an ASCII file.
        save(filename: str) -> str:
            Save the OPAT file as a binary file.
    """
    def __init__(self):
        self.header: Header = make_default_header()
        self.tables: List[Tuple[Tuple[float, float], OPATTable]] = []

    @staticmethod
    def validate_char_array_size(s: str, nmax: int) -> bool:
        """
        @brief Validate the size of a character array.
        @param s The string to validate.
        @param nmax The maximum allowed size.
        @return True if the string size is valid, False otherwise.
        """
        if len(s) > nmax:
            return False
        return True

    @staticmethod
    def validate_logKappa(logKappa):
        """
        @brief Validate the logKappa array.
        @param logKappa The logKappa array to validate.
        @throws ValueError if logKappa is not a non-empty 2D array.
        @throws TypeError if logKappa is not a 2D array or iterable.
        """
        if isinstance(logKappa, np.ndarray):
            if logKappa.ndim == 2:
                return
            else:
                raise ValueError("logKappa must be a non-empty 2D array")

        if isinstance(logKappa, collectionIterable) and all(isinstance(row, collectionIterable) for row in logKappa):
            try:
                first_row = next(iter(logKappa))
                if all(isinstance(x, (int, float)) for x in first_row):
                    return
                else:
                    raise ValueError("logKappa must be fully numeric")
            except StopIteration:
                raise ValueError("logKappa must be a non-empty 2D iterable")
        else:
            raise TypeError("logKappa must be a non-empty 2D array or iterable")

    @staticmethod
    def validate_1D(arr, name: str):
        """
        @brief Validate a 1D array.
        @param arr The array to validate.
        @param name The name of the array.
        @throws ValueError if the array is not 1D or not fully numeric.
        @throws TypeError if the array is not a non-empty 1D array or iterable.
        """
        if isinstance(arr, np.ndarray):
            if arr.ndim == 1:
                return
            else:
                raise ValueError(f"{name} must be a 1D numpy array")
        if isinstance(arr, collectionIterable) and not isinstance(arr, (str, bytes)):
            if all(isinstance(x, (int, float)) for x in arr):
                return
            else:
                raise ValueError(f"{name} must be fully numeric")
        else:
            raise TypeError(f"{name} must be a non-empty 1D array or iterable")

    @staticmethod
    def compute_checksum(data: np.ndarray) -> bytes:
        """
        @brief Compute the SHA-256 checksum of the given data.
        @param data The data to compute the checksum for.
        @return The SHA-256 checksum.
        """
        return hashlib.sha256(data.tobytes()).digest()

    def set_version(self, version: int) -> int:
        """
        @brief Set the version of the OPAT file.
        @param version The version to set.
        @return The set version.
        """
        self.header.version = version
        return self.header.version

    def set_source(self, source: str) -> str:
        """
        @brief Set the source information of the OPAT file.
        @param source The source information to set.
        @return The set source information.
        @throws TypeError if the source string is too long.
        """
        if not self.validate_char_array_size(source, 64):
            raise TypeError(f"sourceInfo string ({source}) is too long ({len(source)}). Max length is 64")
        self.header.sourceInfo = source
        return self.header.sourceInfo

    def set_comment(self, comment: str) -> str:
        """
        @brief Set the comment of the OPAT file.
        @param comment The comment to set.
        @return The set comment.
        @throws TypeError if the comment string is too long.
        """
        if not self.validate_char_array_size(comment, 128):
            raise TypeError(f"comment string ({comment}) is too long ({len(comment)}). Max length is 128")
        self.header.comment = comment
        return self.header.comment
    
    def set_numIndex(self, numIndex: int) -> int:
        """
        @brief Set the number of values to use when indexing table.
        @param numIndex The number of values to use when indexing table.
        @return The set number of values to use when indexing table.
        """
        if numIndex < 1:
            raise ValueError(f"numIndex must be greater than 0! It is currently {numIndex}")
        self.header.numIndex = numIndex
        return self.header.numIndex

    def add_table(self, indicies: Tuple[float], logR: Iterable[float], logT: Iterable[float], logKappa: Iterable[Iterable[float]]):
        """
        @brief Add a table to the OPAT file.
        @param indicies The index values of the table.
        @param logR The logR values.
        @param logT The logT values.
        @param logKappa The logKappa values.
        @throws ValueError if logKappa is not a non-empty 2D array or if logR and logT are not 1D arrays.
        """
        if len(indicies) != self.header.numIndex:
            raise ValueError(f"indicies must have length {self.header.numIndex}! Currently it has length {len(indicies)}")
        self.validate_logKappa(logKappa)
        self.validate_1D(logR, "logR")
        self.validate_1D(logT, "logT")

        logR = np.array(logR)
        logT = np.array(logT)
        logKappa = np.array(logKappa)

        if logKappa.shape != (logR.shape[0], logT.shape[0]):
            raise ValueError(f"logKappa must be of shape ({len(logR)} x {len(logT)})! Currently logKappa has shape {logKappa.shape}")

        table = OPATTable(
            N_R = logR.shape[0],
            N_T = logT.shape[0],
            logR = logR,
            logT = logT,
            logKappa = logKappa
        )
    
        self.tables.append((indicies, table))
        self.header.numTables += 1


    def _header_bytes(self) -> bytes:
        """
        @brief Convert the header to bytes.
        @return The header as bytes.
        """
        headerBytes = struct.pack(
            "<4s H I I Q 16s 64s 128s H 24s",
            self.header.magic.encode('utf-8'),
            self.header.version,
            self.header.numTables,
            self.header.headerSize,
            self.header.indexOffset,
            self.header.creationDate.encode('utf-8'),
            self.header.sourceInfo.encode('utf-8'),
            self.header.comment.encode('utf-8'),
            self.header.numIndex,
            self.header.reserved
        )
        return headerBytes

    def _table_bytes(self, table: OPATTable) -> Tuple[bytes, bytes]:
        """
        @brief Convert a table to bytes.
        @param table The OPAT table.
        @return A tuple containing the checksum and the table as bytes.
        """
        logR = table.logR.flatten()
        logT = table.logT.flatten()
        logKappa = table.logKappa.flatten()
        tableBytes = struct.pack(
            f"<II{table.N_R}d{table.N_T}d{table.N_R*table.N_T}d",
            table.N_R,
            table.N_T,
            *logR,
            *logT,
            *logKappa
        )
        checksum = self.compute_checksum(logKappa)
        return (checksum, tableBytes)

    def _tableIndex_bytes(self, tableIndex: TableIndex) -> bytes:
        """
        @brief Convert a table index to bytes.
        @param tableIndex The table index.
        @return The table index as bytes.
        @throws RuntimeError if the table index entry does not have 64 bytes.
        """
        tableIndexFMTString = "<"+"d"*self.header.numIndex+f"QQ"
        tableIndexBytes = struct.pack(
            tableIndexFMTString,
            *tableIndex.index,
            tableIndex.byteStart,
            tableIndex.byteEnd
        )
        tableIndexBytes += tableIndex.sha256

        if len(tableIndexBytes) != 16+self.header.numIndex*8+32:
            raise RuntimeError(f"Each table index entry must have 64 bytes. Due to an unknown error the table index entry for (X,Z)=({tableIndex.X},{tableIndex.Z}) header has {len(tableIndexBytes)} bytes")
        
        return tableIndexBytes

    def __repr__(self) -> str:
        """
        @brief Get the string representation of the OpatIO object.
        @return The string representation.
        """
        reprString = f"""OpatIO(
  version: {self.header.version}
  numTables: {self.header.numTables}
  headerSize: {self.header.headerSize}
  indexOffset: {self.header.indexOffset}
  creationDate: {self.header.creationDate}
  sourceInfo: {self.header.sourceInfo}
  comment: {self.header.comment}
  numIndex: {self.header.numIndex}
  reserved: {self.header.reserved}
)"""
        return reprString

    def _format_table_as_string(self, table: OPATTable, indices: List[float]) -> str:
        """
        @brief Format a table as a string.
        @param table The OPAT table.
        @indices The index values of the table.
        @return The formatted table as a string.
        """
        tableString: List[str] = []
        # fixed width X and Z header per table
        tableIndexString: List[str] = []
        for index in indices:
            tableIndexString.append(f"{index:<10.4f}")
        tableString.append(" ".join(tableIndexString))
        tableString.append("-" * 80)
        # write logR across the top (reserving one col for where logT will be)
        tableString.append(f"{'':<10}{'logR':<10}")
        logRRow = f"{'logT':<10}"
        logRRowTrue = "".join(f"{r:<10.4f}" for r in table.logR)
        tableString.append(logRRow + logRRowTrue)
        for i, logT in enumerate(table.logT):
            row = f"{logT:<10.4f}"
            for kappa in table.logKappa[:, i]:
                row += f"{kappa:<10.4f}"
            tableString.append(row)
        tableString.append("=" * 80)
        return '\n'.join(tableString)

    @staticmethod
    def print_table_indexes(table_indexes: List[TableIndex]) -> str:
        """
        @brief Print the table indexes.
        @param table_indexes The list of table indexes.
        @return The formatted table indexes as a string.
        """
        if not table_indexes:
            print("No table indexes found.")
            return

        tableRows: List[str] = []
        tableRows.append("\nTable Indexes in OPAT File:\n")
        headerString: str = ''
        for indexID, index in enumerate(table_indexes[0].index):
            indexKey = f"Index {indexID}"
            headerString += f"{indexKey:<10}"
        headerString += f"{'Byte Start':<15} {'Byte End':<15} {'Checksum (SHA-256)'}"
        tableRows.append(headerString)
        tableRows.append("=" * 80)
        for entry in table_indexes:
            tableEntry = ''
            for index in entry.index:
                tableEntry += f"{index:<10.4f}"
            tableEntry += f"{entry.byteStart:<15} {entry.byteEnd:<15} {entry.sha256[:16]}..."
            tableRows.append(tableEntry)
        return '\n'.join(tableRows)

    def save_as_ascii(self, filename: str) -> str:
        """
        @brief Save the OPAT file as an ASCII file.
        @param filename The name of the file.
        @return The name of the saved file.
        """
        numericFMT = "{:.18e}"
        currentStartByte: int = 256
        tableIndexs: List[bytes] = []
        tableStrings: List[bytes] = []
        for index, table in self.tables:
            checksum, tableBytes = self._table_bytes(table)
            tableStrings.append(self._format_table_as_string(table, index) + "\n")
            tableIndex = TableIndex(
                index = index,
                byteStart = currentStartByte,
                byteEnd = currentStartByte + len(tableBytes),
                sha256 = checksum
            )
            tableIndexs.append(tableIndex)
            

            currentStartByte += len(tableBytes)
        self.header.indexOffset = currentStartByte
        with open(filename, 'w') as f:
            f.write("This is an ASCII representation of an OPAT file, it is not a valid OPAT file in and of itself.\n")
            f.write("This file is meant to be human readable and is not meant to be read by a computer.\n")
            f.write("The purpose of this file is to provide a human readable representation of the OPAT file which can be used for debugging purposes.\n")
            f.write("The full binary specification of the OPAT file can be found in the OPAT file format documentation at:\n")
            f.write(" https://github.com/4D-STAR/4DSSE/blob/main/specs/OPAT/OPAT.pdf\n")
            f.write("="*35 + " HEADER " + "="*36 + "\n")
            f.write(f">> {self.header.magic}\n")
            f.write(f">> Version: {self.header.version}\n")
            f.write(f">> numTables: {self.header.numTables}\n")
            f.write(f">> headerSize (bytes): {self.header.headerSize}\n")
            f.write(f">> tableIndex Offset (bytes): {self.header.indexOffset}\n")
            f.write(f">> Creation Date: {self.header.creationDate}\n")
            f.write(f">> Source Info: {self.header.sourceInfo}\n")
            f.write(f">> Comment: {self.header.comment}\n")
            f.write(f">> numIndex: {self.header.numIndex}\n")
            f.write("="*37 + " DATA " + "="*37 + "\n")
            f.write("="*80 + "\n")
            for tableString in tableStrings:
                f.write(tableString)
                f.write("="*80 + "\n")
            f.write("="*36 + " INDEX " + "="*37 + "\n")
            f.write(self.print_table_indexes(tableIndexs))

    def save(self, filename: str) -> str:
        """
        @brief Save the OPAT file as a binary file.
        @param filename The name of the file.
        @return The name of the saved file.
        @throws RuntimeError if the header does not have 256 bytes.
        """
        tempHeaderBytes = self._header_bytes()

        if len(tempHeaderBytes) != 256:
            raise RuntimeError(f"Header must have 256 bytes. Due to an unknown error the header has {len(tempHeaderBytes)} bytes")

        currentStartByte: int = 256
        tableIndicesBytes: List[bytes] = []
        tablesBytes: List[bytes] = []
        for index, table in self.tables:
            checksum, tableBytes = self._table_bytes(table)
            tableIndex = TableIndex(
                index,
                byteStart = currentStartByte,
                byteEnd = currentStartByte + len(tableBytes),
                sha256 = checksum
            )
            tableIndexBytes = self._tableIndex_bytes(tableIndex)
            tablesBytes.append(tableBytes)
            tableIndicesBytes.append(tableIndexBytes)

            currentStartByte += len(tableBytes)
        self.header.indexOffset = currentStartByte
        headerBytes = self._header_bytes()

        with open(filename, 'wb') as f:
            f.write(headerBytes)
            for tableBytes in tablesBytes:
                f.write(tableBytes)
            for tableIndexBytes in tableIndicesBytes:
                f.write(tableIndexBytes)

        if os.path.exists(filename):
            return filename


def loadOpat(filename: str) -> OpatIO:
    """
    @brief Load an OPAT file.
    @param filename The name of the file.
    @return The loaded OpatIO object.
    @throws RuntimeError if the header does not have 256 bytes.
    """
    opat = OpatIO()
    with open(filename, 'rb') as f:
        headerBytes: bytes = f.read(256)
        unpackedHeader = struct.unpack("<4s H I I Q 16s 64s 128s H 24s", headerBytes)
        loadedHeader = Header(
            magic = unpackedHeader[0].decode().replace("\x00", ""),
            version = unpackedHeader[1],
            numTables = unpackedHeader[2],
            headerSize = unpackedHeader[3],
            indexOffset = unpackedHeader[4],
            creationDate = unpackedHeader[5].decode().replace("\x00", ""),
            sourceInfo = unpackedHeader[6].decode().replace("\x00", ""),
            comment = unpackedHeader[7].decode().replace("\x00", ""),
            numIndex = unpackedHeader[8],
            reserved = unpackedHeader[9]
        )
        opat.header = loadedHeader
        f.seek(opat.header.indexOffset)
        tableIndices: List[TableIndex] = []
        tableIndexChunkSize = 16 + loadedHeader.numIndex*8
        tableIndexFMTString = "<"+"d"*loadedHeader.numIndex+"QQ"
        while tableIndexEntryBytes := f.read(tableIndexChunkSize):
            unpackedTableIndexEntry = struct.unpack(tableIndexFMTString, tableIndexEntryBytes)
            checksum = f.read(32)
            index = unpackedTableIndexEntry[:loadedHeader.numIndex]
            tableIndexEntry = TableIndex(
                index = index,
                byteStart = unpackedTableIndexEntry[loadedHeader.numIndex],
                byteEnd = unpackedTableIndexEntry[loadedHeader.numIndex+1],
                sha256 = checksum
            )
            tableIndices.append(tableIndexEntry)
        
        currentStartByte = opat.header.headerSize
        f.seek(currentStartByte)
        for tableIndex in tableIndices:
            f.seek(tableIndex.byteStart)
            byteLength = tableIndex.byteEnd - tableIndex.byteStart
            tableBytes = f.read(byteLength)

            nr_nt_fmt = "<II"
            nr_nt_size = struct.calcsize(nr_nt_fmt)
            N_R, N_T = struct.unpack(nr_nt_fmt, tableBytes[:nr_nt_size])

            dataFormat = f"<{N_R}d{N_T}d{N_R*N_T}d"
            unpackedData = struct.unpack(dataFormat, tableBytes[nr_nt_size:])

            logR = np.array(unpackedData[:N_R], dtype=np.float64)
            logT = np.array(unpackedData[N_R: N_R+N_T], dtype=np.float64)
            logKappa = np.array(unpackedData[N_R+N_T:], dtype=np.float64).reshape((N_R, N_T))

            opat.add_table(tableIndex.index, logR, logT, logKappa)
    return opat
