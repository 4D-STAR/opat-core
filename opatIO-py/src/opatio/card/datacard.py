from dataclasses import dataclass
import struct
import hashlib
import numpy as np
import numpy.typing as npt

from typing import Dict, Iterable, Tuple, Union

from opatio.misc.opatentity import OPATEntity


@dataclass
class CardHeader(OPATEntity):
    """
    Represents the header of a data card.

    Attributes
    ----------
    numTables : int
        Number of tables in the data card.
    indexOffset : int
        Offset to the index section in bytes.
    cardSize : int
        Total size of the data card in bytes.
    comment : str
        Comment section of the header.
    reserved : bytes
        Reserved for future use (default is 100 null bytes).
    magicNumber : str
        Magic number to validate the data card (default is "CARD").
    headerSize : int
        Fixed size of the data card header (default is 256 bytes).
    """

    numTables: int
    indexOffset: int
    cardSize: int
    comment: str
    reserved: bytes = b"\x00"*100
    magicNumber: str = "CARD"
    headerSize: int = 256

    def set_comment(self, comment: str):
        """
        Set the comment of the data card.

        Parameters
        ----------
        comment : str
            The comment to set.

        Raises
        ------
        ValueError
            If the comment string exceeds 128 characters.

        Examples
        --------
        >>> header = CardHeader(numTables=0, indexOffset=0, cardSize=0, comment="")
        >>> header.set_comment("This is a comment.")
        """
        if len(comment) >= 128:
            raise ValueError(f"comment string ({comment}) is too long ({len(comment)}). Max length is 128")
        self.comment = comment

    def __bytes__(self) -> bytes:
        """
        Convert the card header to bytes.

        Returns
        -------
        bytes
            The card header as bytes.

        Raises
        ------
        AssertionError
            If the header size is not 256 bytes.

        Examples
        --------
        >>> header = CardHeader(numTables=1, indexOffset=256, cardSize=512, comment="Example")
        >>> bytes(header)
        """
        headerBytes = struct.pack(
            "<4s I I Q Q 128s 100s",
            self.magicNumber.encode('utf-8'),
            self.numTables,
            self.headerSize,
            self.indexOffset,
            self.cardSize,
            self.comment.encode('utf-8'),
            self.reserved
        )
        assert len(headerBytes) == 256, f"Header must be 256 bytes. Due to an unknown error the header has {len(headerBytes)} bytes"
        return headerBytes

    def __repr__(self) -> str:
        """
        Get the string representation of the card header.

        Returns
        -------
        str
            The string representation.

        Examples
        --------
        >>> header = CardHeader(numTables=1, indexOffset=256, cardSize=512, comment="Example")
        >>> repr(header)
        'CardHeader(magicNumber=CARD, numTables=1, headerSize=256, indexOffset=256, cardSize=512, comment=Example)'
        """
        return f"CardHeader(magicNumber={self.magicNumber}, numTables={self.numTables}, headerSize={self.headerSize}, indexOffset={self.indexOffset}, cardSize={self.cardSize}, comment={self.comment})"

    def ascii(self) -> str:
        """
        Get the ASCII representation of the card header.

        Returns
        -------
        str
            The ASCII representation.

        Examples
        --------
        >>> header = CardHeader(numTables=1, indexOffset=256, cardSize=512, comment="Example")
        >>> print(header.ascii())
        ========== Card Header ==========
        >> Magic Number: CARD
        >> Number of Tables: 1
        >> Header Size: 256
        >> Index Offset: 256
        >> Card Size: 512
        >> Comment: Example
        """
        asciiString = f"""========== Card Header ==========
>> Magic Number: {self.magicNumber}
>> Number of Tables: {self.numTables}
>> Header Size: {self.headerSize}
>> Index Offset: {self.indexOffset}
>> Card Size: {self.cardSize}
>> Comment: {self.comment}
"""
        return asciiString

    def copy(self):
        """
        Create a copy of the card header.

        Returns
        -------
        CardHeader
            A copy of the card header.

        Examples
        --------
        >>> header = CardHeader(numTables=1, indexOffset=256, cardSize=512, comment="Example")
        >>> header_copy = header.copy()
        >>> header_copy == header
        True
        """
        return CardHeader(
            numTables=self.numTables,
            indexOffset=self.indexOffset,
            cardSize=self.cardSize,
            comment=self.comment,
            reserved=self.reserved
        )

@dataclass
class CardIndexEntry(OPATEntity):
    """
    Represents an entry in the index of a data card.

    Attributes
    ----------
    tag : str
        Tag to identify the table.
    byteStart : int
        Byte start position of the table relative to the start of the card.
    byteEnd : int
        Byte end position of the table relative to the start of the card.
    numColumns : int
        Number of columns in the table.
    numRows : int
        Number of rows in the table.
    columnName : str
        Name of the column.
    rowName : str
        Name of the row.
    size : int
        Length of the row entry (default is 1). Maximum is 2^64 - 1.
    reserved : bytes
        Reserved for future use (default is 12 null bytes).
    """

    tag: str
    byteStart: int
    byteEnd: int
    numColumns: int
    numRows: int
    columnName: str
    rowName: str
    size: int = 1
    reserved: bytes = b"\x00"*12

    def __bytes__(self) -> bytes:
        """
        Convert the card index to bytes.

        Returns
        -------
        bytes
            The card index as bytes.

        Raises
        ------
        AssertionError
            If the index entry size is not 64 bytes.

        Examples
        --------
        >>> index = CardIndexEntry(tag="Example", byteStart=0, byteEnd=64, numColumns=4, numRows=4, columnName="col", rowName="row")
        >>> bytes(index)
        """
        nullPaddedTag = self.tag.ljust(8, '\x00').encode('utf-8')
        nullPaddedColumnName = self.columnName.ljust(8, '\x00').encode('utf-8')
        nullPaddedRowName = self.rowName.ljust(8, '\x00').encode('utf-8')
        if not self.size.is_integer():
            raise TypeError(f"Due to an unknown error the size of the index entry is not an integer. The size is {self.size}. This is a opatio bug and should be reported.")
        indexBytes = struct.pack(
            f"<8s Q Q H H 8s 8s Q 12s",
            nullPaddedTag,
            self.byteStart,
            self.byteEnd,
            self.numColumns,
            self.numRows,
            nullPaddedColumnName,
            nullPaddedRowName,
            int(self.size),
            self.reserved
        )
        assert len(indexBytes) == 64, f"Card index entry must be 64 bytes. Due to an unknown error the card index entry has {len(indexBytes)} bytes"
        return indexBytes

    def __repr__(self) -> str:
        """
        Get the string representation of the card index.

        Returns
        -------
        str
            The string representation.

        Examples
        --------
        >>> index = CardIndexEntry(tag="Example", byteStart=0, byteEnd=64, numColumns=4, numRows=4, columnName="col", rowName="row", size=1)
        >>> repr(index)
        'CardIndexEntry(Tag=Example, byteStart=0, byteEnd=64, numColumns=4, numRows=4, columnName=col, rowName=row, size=1)'
        """
        return f"CardIndexEntry(Tag={self.tag}, byteStart={self.byteStart}, byteEnd={self.byteEnd}, numColumns={self.numColumns}, numRows={self.numRows}, columnName={self.columnName}, rowName={self.rowName}, size={self.size})"

    def ascii(self) -> str:
        """
        Get the ASCII representation of the card index.

        Returns
        -------
        str
            The ASCII representation.

        Examples
        --------
        >>> index = CardIndexEntry(tag="Example", byteStart=0, byteEnd=64, numColumns=4, numRows=4, columnName="col", rowName="row", size=1)
        >>> print(index.ascii())
        Example  |        0 |       64 |        4 |        4 | col      | row       | 1
        """
        return f"{self.tag:8} | {self.byteStart:8} | {self.byteEnd:8} | {self.numColumns:8} | {self.numRows:8} | {self.columnName:8} | {self.rowName:8} | {self.size:8}\n"

    def copy(self):
        """
        Create a copy of the card index entry.

        Returns
        -------
        CardIndexEntry
            A copy of the card index entry.

        Examples
        --------
        >>> index = CardIndexEntry(tag="Example", byteStart=0, byteEnd=64, numColumns=4, numRows=4, columnName="col", rowName="row")
        >>> index_copy = index.copy()
        >>> index_copy == index
        True
        """
        return CardIndexEntry(
            tag=self.tag,
            byteStart=self.byteStart,
            byteEnd=self.byteEnd,
            numColumns=self.numColumns,
            numRows=self.numRows,
            columnName=self.columnName,
            rowName=self.rowName,
            size = self.size,
            reserved=self.reserved
        )

@dataclass
class OPATTable(OPATEntity):
    """
    Represents the data of a single table in an OPAT file.

    Attributes
    ----------
    columnValues : Iterable[float]
        Column values of the table.
    rowValues : Iterable[float]
        Row values of the table.
    data : npt.ArrayLike
        Data of the table, stored as a 2D or 3D array.

    Notes
    -----
    - This class is primarily used internally by opatio. Advanced users may
      create their own tables using this class if needed.
    """

    columnValues: npt.ArrayLike
    rowValues: npt.ArrayLike
    data: npt.ArrayLike
    _size: int = ...

    def __post_init__(self):
        """
        Perform post-initialization checks and set the size attribute based on the data's dimensions.

        Raises
        ------
        ValueError
            If the data is not a 2D or 3D array.
        """
        if self.data.ndim != 2 and self.data.ndim != 3:
           raise ValueError(f"data must be a 2D or 3D array! Currently it is {self.data.ndim}D")

        if self.data.ndim == 2:
            self._size = 1
        elif self.data.ndim == 3:
            self._size = self.data.shape[2]

    @property
    def size(self) -> int:
        """
        Get the size of the table, which is 1 for 2D data or the third dimension for 3D data.

        Returns
        -------
        int
            The size of the table.
        """
        return self._size

    def __getitem__(self, key: Union[Tuple[int, int], Tuple[int, int, int]]):
        """
        Retrieve a specific element or slice from the table.

        Parameters
        ----------
        key : tuple
            A tuple of indices specifying the element or slice to retrieve.

        Returns
        -------
        Any
            The requested element or slice.

        Raises
        ------
        TypeError
            If the key is not a tuple.
        KeyError
            If the key is not a tuple of integers or has an invalid length.
        """
        if not isinstance(key, tuple):
            raise TypeError(f"key must be a tuple! Currently it is {type(key)}")
        if not all([isinstance(x, int) for x in key]):
            raise KeyError(f"key must be a tuple of integers! Currently it is {key}")
        if len(key) != 2 and len(key) != 3:
            raise KeyError(f"key must be a tuple of length 2 or 3! Currently it is {len(key)}")
        return self.data[*key]

    def sha256(self) -> "_Hash":
        """
        Compute the SHA-256 checksum of the given data.

        Returns
        -------
        bytes
            The SHA-256 checksum.

        Raises
        ------
        ValueError
            If the data cannot be cast to a numpy array.

        Examples
        --------
        >>> table = OPATTable(columnValues=[1.0, 2.0], rowValues=[3.0, 4.0], data=[[5.0, 6.0], [7.0, 8.0]])
        >>> table.sha256()
        """
        flatData = self.data.flatten()
        return hashlib.sha256(flatData.tobytes())

    def __bytes__(self) -> bytes:
        """
        Convert the single OPAT format table to bytes.

        Returns
        -------
        bytes
            The OPAT table as bytes.

        Raises
        ------
        ValueError
            If columnValues, rowValues, or data cannot be cast to numpy arrays.

        AssertionError
            If the byte sizes of columnValues, rowValues, or data are incorrect.

        Examples
        --------
        >>> table = OPATTable(columnValues=[1.0, 2.0], rowValues=[3.0, 4.0], data=[[5.0, 6.0], [7.0, 8.0]])
        >>> bytes(table)
        """
        if not isinstance(self.columnValues, np.ndarray):
            try:
                cV = np.array(self.columnValues, dtype=np.float64).flatten()
            except ValueError as e:
                raise ValueError(f"columnValues must be castable to a numpy array! Currently it is {type(self.columnValues)}. {e}")
        else:
            cV = self.columnValues.flatten()
        if not isinstance(self.rowValues, np.ndarray):
            try:
                rV = np.array(self.rowValues, dtype=np.float64).flatten()
            except ValueError as e:
                raise ValueError(f"rowValues must be castable to a numpy array! Currently it is {type(self.rowValues)}. {e}")
        else:
            rV = self.rowValues.flatten()
        if not isinstance(self.data, np.ndarray):
            try:
                data = np.array(self.data, dtype=np.float64).flatten()
            except ValueError as e:
                raise ValueError(f"data must be castable to a numpy array! Currently it is {type(self.data)}. {e}")
        else:
            data = self.data.flatten()

        tableBytes = struct.pack(
            f"<{len(rV)}d{len(cV)}d{len(data)}d",
            *rV,
            *cV,
            *data
        )
        return tableBytes


    @staticmethod
    def compute_col_width(size: int, floatWidth: int) -> int:
        """
        Compute the total width of a cell for ASCII representation.

        Parameters
        ----------
        size : int
            Number of floats in the cell.
        floatWidth : int
            Width of each float.

        Returns
        -------
        int
            Total width of the cell.
        """
        # Width per float plus the separator ", "
        floatWidth = len(f"{0:7.{floatWidth}f}")
        perFloat = floatWidth
        separatorWidth = 2 * (size - 1) if size > 1 else 0
        # Include 2 for the surrounding <>
        bracketWidth = 2 if size > 1 else 0
        return int(size * perFloat + separatorWidth + bracketWidth)

    def format_centered(self, value: float, floatWidth: int) -> str:
        """
        Format a float value centered in a fixed-width field.

        Parameters
        ----------
        value : float
            The value to format.
        floatWidth : int
            The width of the float.

        Returns
        -------
        str
            The formatted string.
        """
        totalWidth = self.compute_col_width(self.size, floatWidth)
        formatSpec = f"^{totalWidth}.{floatWidth}f"
        return f"{value:{formatSpec}}"

    def ascii(self) -> str:
        """
        Get the ASCII representation of the OPAT table.

        Returns
        -------
        str
            The ASCII representation.

        Examples
        --------
        >>> table = OPATTable(columnValues=[1.0, 2.0], rowValues=[3.0, 4.0], data=[[5.0, 6.0], [7.0, 8.0]])
        >>> print(table.ascii())
        """
        tableStr = ""
        numRows = len(self.rowValues)
        numColumns = len(self.columnValues)
        # colNameRow = " | ".join([f"{col:4.4f}" for col in self.columnValues])
        colNameRow = " | ".join([self.format_centered(col, 4) for col in self.columnValues])
        colNameRow = "        | " + colNameRow
        tableStr += colNameRow + "\n"
        tableStr += "-" * (len(colNameRow) + 4) + "\n"
        for i in range(numRows):
            cellValues = [self.data[i][j] for j in range(numColumns)]
            if isinstance(cellValues[0], np.ndarray):
                dataSequence = [', '.join([f"{y:7.4f}" for y in x]) for x in cellValues]
                dataVectorFormated = [f'<{x}>' if ',' in x else x for x in dataSequence]
            else:
                dataVectorFormated = [f"{x:7.4f}" for x in cellValues]

            row = " | ".join(dataVectorFormated)
            tableStr += f"{self.rowValues[i]:7.4f} | " + row + "\n"
        return tableStr

    def copy(self):
        """
        Create a copy of the OPAT table.

        Returns
        -------
        OPATTable
            A copy of the OPAT table.

        Examples
        --------
        >>> table = OPATTable(columnValues=[1.0, 2.0], rowValues=[3.0, 4.0], data=[[5.0, 6.0], [7.0, 8.0]])
        >>> table_copy = table.copy()
        >>> table_copy == table
        True
        """
        newTable = OPATTable(
            columnValues=self.columnValues.copy(),
            rowValues=self.rowValues.copy(),
            data=self.data.copy()
        )
        return newTable

    def __repr__(self):
        """
        Get the string representation of the OPAT table.

        Returns
        -------
        str
            The string representation.
        """
        outStr = "OPATTable("
        outStr += f"columnValues: [{self.columnValues.min():0.4f} -> {self.columnValues.max():0.4f}], "
        outStr += f"rowValues: [{self.rowValues.min():0.4f} -> {self.rowValues.max():0.4f}], "
        outStr += f"vSize={self.size})"
        return outStr

@dataclass
class DataCard(OPATEntity):
    """
    Represents a data card containing a header, index, and tables.

    Attributes
    ----------
    header : CardHeader
        Header of the data card.
    index : Dict[str, CardIndexEntry]
        Index of the data card, mapping tags to index entries.
    tables : Dict[str, OPATTable]
        Tables in the data card, mapped by their tags.

    Methods
    -------
    add_table(tag: str, table: OPATTable, columnName: str = "columnValues", rowName: str = "rowValues")
        Add a table to the data card.
    sha256() -> bytes
        Compute the SHA-256 hash of the data card, including all tables and their data.
    ascii() -> str
        Get the ASCII representation of the data card.
    copy() -> DataCard
        Create a deep copy of the data card.
    """

    header: CardHeader
    index: Dict[str, CardIndexEntry]
    tables: Dict[str, OPATTable]

    def __init__(self):
        """
        Initialize a DataCard instance with default header, index, and tables.
        """
        self.header = CardHeader(numTables=0, indexOffset=256, cardSize=256, comment="")
        self.index = {}
        self.tables = {}

    def add_table(self, tag: str, table: OPATTable, columnName: str = "columnValues", rowName: str = "rowValues"):
        """
        Add a table to the data card.

        Parameters
        ----------
        tag : str
            Tag to identify the table.
        table : OPATTable
            The table to add.
        columnName : str, optional
            Name of the column (default is "columnValues").
        rowName : str, optional
            Name of the row (default is "rowValues").

        Raises
        ------
        TypeError
            If table is not an OPATTable or tag is not a string.
        ValueError
            If a table with the same tag already exists.

        Examples
        --------
        >>> card = DataCard()
        >>> table = OPATTable(columnValues=[1.0, 2.0], rowValues=[3.0, 4.0], data=[[5.0, 6.0], [7.0, 8.0]])
        >>> card.add_table(tag="Example", table=table)
        """
        if not isinstance(table, OPATTable):
            raise TypeError(f"table must be an OPATTable! Currently it is {type(table)}")

        if not isinstance(tag, str):
            raise TypeError(f"tag must be a string! Currently it is {type(tag)}")

        if tag in self.index:
            raise ValueError(f"Table with tag {tag} already exists in the card!")

        # Add the table to the data card
        self.tables[tag] = table

        byteStart = max([entry.byteEnd for entry in self.index.values()], default=256)

        # Create the index entry for the table
        index = CardIndexEntry(
            tag=tag,
            byteStart=byteStart,
            byteEnd=byteStart + len(table),
            numColumns=len(table.columnValues),
            numRows=len(table.rowValues),
            columnName=columnName,
            rowName=rowName,
            size=table.size
        )

        # Add the index entry to the data card
        self.index[tag] = index

        # Update the header information
        self.header.numTables += 1
        cardSize = 256
        indexOffset = 256
        for tag in self.tables:
            cardSize += len(self.tables[tag])
            cardSize += len(self.index[tag])
            indexOffset += len(self.tables[tag])
        self.header.cardSize = cardSize
        self.header.indexOffset = indexOffset

    def sha256(self) -> bytes:
        """
        Compute the SHA-256 hash of the data card.

        Returns
        -------
        bytes
            The SHA-256 hash of the data card.

        Examples
        --------
        >>> card = DataCard()
        >>> table = OPATTable(columnValues=[1.0, 2.0], rowValues=[3.0, 4.0], data=[[5.0, 6.0], [7.0, 8.0]])
        >>> card.add_table(tag="Example", table=table)
        >>> card.sha256()
        """
        sha256 = hashlib.sha256()
        for _, table in self.tables.items():
            sha256.update(table.sha256().digest())

        return sha256.digest()

    def __bytes__(self) -> bytes:
        """
        Convert the entire data card to bytes, including header, tables, and index.

        Returns
        -------
        bytes
            The data card as bytes.

        Examples
        --------
        >>> card = DataCard()
        >>> table = OPATTable(columnValues=[1.0, 2.0], rowValues=[3.0, 4.0], data=[[5.0, 6.0], [7.0, 8.0]])
        >>> card.add_table(tag="Example", table=table)
        >>> bytes(card)
        """
        headerBytes = bytes(self.header)
        indexBytes = b"".join(bytes(index) for _, index in self.index.items())
        tablesBytes = b"".join(bytes(table) for _, table in self.tables.items())
        return headerBytes + tablesBytes + indexBytes

    def __getitem__(self, key: str) -> OPATTable:
        """
        Get the table by index.

        Parameters
        ----------
        key : str
            The index of the table.

        Returns
        -------
        OPATTable
            The table.

        Raises
        ------
        TypeError
            If key is not a string.
        KeyError
            If the table with the given index is not found.

        Examples
        --------
        >>> card = DataCard()
        >>> table = OPATTable(columnValues=[1.0, 2.0], rowValues=[3.0, 4.0], data=[[5.0, 6.0], [7.0, 8.0]])
        >>> card.add_table(tag="Example", table=table)
        >>> card["Example"]
        """
        if not isinstance(key, str):
            raise TypeError(f"key must be a string! Currently it is {type(key)}")
        if key not in self.index:
            raise KeyError(f"Table with index {key} not found.")
        return self.tables[key].copy()

    def __repr__(self) -> str:
        """
        Get the string representation of the data card.

        Returns
        -------
        str
            The string representation.

        Examples
        --------
        >>> card = DataCard()
        >>> repr(card)
        'DataCard(numTables=0, indexOffset=256, cardSize=256, comment=)'
        """
        reprString = f"""DataCard(numTables={self.header.numTables}, indexOffset={self.header.indexOffset}, cardSize={len(self)}, comment={self.header.comment})"""
        return reprString

    def ascii(self) -> str:
        """
        Get the ASCII representation of the data card.

        Returns
        -------
        str
            The ASCII representation.

        Examples
        --------
        >>> card = DataCard()
        >>> print(card.ascii())
        """
        asciiRepr = "======== START Data Card ========\n"
        asciiRepr += self.header.ascii()

        asciiRepr += "======== Tables ========\n"
        for i, (tag, table) in enumerate(self.tables.items()):
            index = self.index[tag]
            asciiRepr += f"-------- Table {i} (tag: {index.tag}) --------\n"
            asciiRepr += table.ascii()

        asciiRepr += "======== Card Index ========\n"
        for tag, index in self.index.items():
            asciiRepr += index.ascii()

        asciiRepr += "========= END Data Card =========\n"
        return asciiRepr

    def copy(self):
        """
        Create a copy of the data card.

        Returns
        -------
        DataCard
            A copy of the data card.

        Examples
        --------
        >>> card = DataCard()
        >>> card_copy = card.copy()
        >>> card_copy == card
        True
        """
        newCard = DataCard()
        newCard.header = self.header.copy()
        newCard.index = {tag: index.copy() for tag, index in self.index.items()}
        newCard.tables = {tag: table.copy() for tag, table in self.tables.items()}
        return newCard
