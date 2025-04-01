from typing import Iterable, List, Dict, Union
import os
import numpy as np

from opatio.base.header import Header, make_default_header
from opatio.index.floatvectorindex import FloatVectorIndex
from opatio.catalog.entry import CardCatalogEntry
from opatio.card.datacard import DataCard
from opatio.card.datacard import OPATTable

class OPAT():
    """
    A class representing an OPAT (Open Parameterized Array Table) instance. OPAT
    is a structured binary file format developed by the 4D-STAR collaboration for
    storing all tabular data needed for 4DSSE. OPAT is liscensed under the
    GNU General Public License v3.0. You should have received a copy of the
    GNU General Public License along with this program. If not, see
    <http://www.gnu.org/licenses/>.

    This class provides methods for managing headers, catalogs, and data cards, 
    as well as saving OPAT files.

    Attributes
    ----------
    header : Header
        The header object containing metadata for the OPAT instance.
    catalog : Dict[FloatVectorIndex, CardCatalogEntry]
        A dictionary mapping index vectors to catalog entries.
    cards : Dict[FloatVectorIndex, DataCard]
        A dictionary mapping index vectors to data cards.

    Methods
    -------
    set_version(version: int) -> int
        Sets the version of the OPAT header.

    set_source(source: str) -> str
        Sets the source information in the OPAT header.

    set_comment(comment: str) -> str
        Sets the comment in the OPAT header.

    set_numIndex(numIndex: int) -> int
        Sets the number of indices in the OPAT header.

    pop_card(indexVector: Union[FloatVectorIndex, Iterable[float]]) -> DataCard
        Removes and returns a card from the catalog and cards dictionary. The
        organization of the file will be updated to reflect the removal of the card.

    add_card(indexVector: Union[FloatVectorIndex, Iterable[float]], card: DataCard)
        Adds a data card to the catalog and cards dictionary. The organization
        of the file will be updated to reflect the addition of the card.

        Notes
        -----
            If a card already exists at the given indexVector, it will first be popped then readded with any
            updates applied

    add_table(indexVector: Union[FloatVectorIndex, Iterable[float]], tag: str, columnValues: Iterable[float], rowValues: Iterable[float], data: Iterable[Iterable[float]], card: DataCard = ..., columnName: str = "columnValues", rowName: str = "rowValues") -> DataCard
        Adds a table to a data card and stores it in the catalog.
        The organization of the file will be updated to reflect the addition of the table.
        The card will be added to the catalog and the organization of the file will be updated to reflect
        the addition of the table.

        Notes
        -----
            If a card already exists at the given indexVector, it will first be popped then readded with any
            updates applied

    save_as_ascii(filename: str) -> str
        Saves the OPAT instance as a human-readable ASCII file. This file is not
        a valid OPAT file in and of itself, but is meant to be human-readable for
        debugging purposes.

    save(filename: str) -> str
        Saves the OPAT instance as a binary file. The file will be saved in the
        specified format and will be a valid OPAT file.


    Examples
    --------
    >>> opat = OPAT()
    >>> opat.set_version(1)
    >>> opat.set_source("example_source")
    >>> opat.set_comment("This is a test comment.")
    """
    def __init__(self):
        """
        Initializes an OPAT instance with default header, catalog, and cards.
        """
        self.header: Header = make_default_header()
        self.catalog: Dict[FloatVectorIndex, CardCatalogEntry] = {}
        self.cards: Dict[FloatVectorIndex, DataCard] = {}

    def set_version(self, version: int) -> int:
        """
        Sets the version of the OPAT header.

        Parameters
        ----------
        version : int
            The version number to set.

        Returns
        -------
        int
            The updated version number.
        """
        self.header.version = version
        return self.header.version

    def set_source(self, source: str) -> str:
        """
        Sets the source information in the OPAT header.

        Parameters
        ----------
        source : str
            The source information to set.

        Returns
        -------
        str
            The updated source information.
        """
        self.header.set_source(source)
        return self.header.sourceInfo

    def set_comment(self, comment: str) -> str:
        """
        Sets the comment in the OPAT header.

        Parameters
        ----------
        comment : str
            The comment to set.

        Returns
        -------
        str
            The updated comment.
        """
        self.header.set_comment(comment)
        return self.header.comment
    
    def set_numIndex(self, numIndex: int) -> int:
        """
        Sets the number of indices in the OPAT header.

        Parameters
        ----------
        numIndex : int
            The number of indices to set. Must be greater than 0.

        Returns
        -------
        int
            The updated number of indices.

        Raises
        ------
        ValueError
            If numIndex is less than 1.
        """
        if numIndex < 1:
            raise ValueError(f"numIndex must be greater than 0! It is currently {numIndex}")
        self.header.numIndex = numIndex
        return self.header.numIndex

    def _validate_indexVector(self, indexVector: Union[FloatVectorIndex, Iterable[float]]) -> FloatVectorIndex:
        """
        Validates and converts an index vector to a FloatVectorIndex.

        Parameters
        ----------
        indexVector : Union[FloatVectorIndex, Iterable[float]]
            The index vector to validate.

        Returns
        -------
        FloatVectorIndex
            The validated and converted index vector.

        Raises
        ------
        ValueError
            If the index vector cannot be cast to a FloatVectorIndex or has an invalid length.
        """
        if not isinstance(indexVector, FloatVectorIndex):
            try:
                indexTuple = tuple(float(i) for i in indexVector)
                indexVector = FloatVectorIndex(
                    vector=indexTuple,
                    hashPrecision=self.header.hashPrecision
                )
            except ValueError as e:
                raise ValueError(f"indexVector must be castable as a tuple of floats or a FloatIndexVector! Currently it is {type(indexVector)}. {e}")
        if len(indexVector) != self.header.numIndex:
            raise ValueError(f"indexVector must have length {self.header.numIndex}! Currently it has length {len(indexVector)}")
        return indexVector

    def _recaclulate_index(self):
        """
        Recalculates the index for all cards in the catalog.

        Updates the catalog and header with the new byte offsets and card counts.
        """
        currentByteStart = self.header.headerSize
        for indexVector, card in self.cards.items():
            currentByteEnd = currentByteStart + len(card)
            self.catalog[indexVector] = CardCatalogEntry(
                index=indexVector,
                byteStart=currentByteStart,
                byteEnd=currentByteEnd,
                sha256=card.sha256()
            )
            currentByteStart = currentByteEnd
            self.header.catalogOffset = currentByteStart
        self.header.numCards = len(self.catalog)

    def pop_card(self, indexVector: Union[FloatVectorIndex, Iterable[float]]) -> DataCard:
        """
        Removes and returns a card from the catalog and cards dictionary.

        Parameters
        ----------
        indexVector : Union[FloatVectorIndex, Iterable[float]]
            The index vector of the card to remove.

        Returns
        -------
        DataCard
            The removed data card.

        Raises
        ------
        KeyError
            If the index vector is not found in the catalog.
        """
        indexVector = self._validate_indexVector(indexVector)
        if indexVector not in self.catalog:
            raise KeyError(f"indexVector {indexVector} not found in catalog!")
        card = self.cards.pop(indexVector)
        self._recaclulate_index()
        return card

    def add_card(self, indexVector: Union[FloatVectorIndex, Iterable[float]], card: DataCard):
        """
        Adds a data card to the catalog and cards dictionary.

        Parameters
        ----------
        indexVector : Union[FloatVectorIndex, Iterable[float]]
            The index vector for the card.
        card : DataCard
            The data card to add.

        Raises
        ------
        TypeError
            If the card is not an instance of DataCard.
        RuntimeError
            If an error occurs while removing an existing card.
        """
        indexVector = self._validate_indexVector(indexVector)

        if not isinstance(card, DataCard):
            raise TypeError(f"card must be a DataCard! Currently it is {type(card)}")

        try:
            self.pop_card(indexVector)
        except KeyError as e:
            pass
        except Exception as e:
            raise RuntimeError(f"Unable to pop card {indexVector}. {e}")

        self.cards[indexVector] = card
        self._recaclulate_index()

    def add_table(
            self,
            indexVector: Union[FloatVectorIndex, Iterable[float]],
            tag: str,
            columnValues: Iterable[float],
            rowValues: Iterable[float],
            data: Iterable[Iterable[float]],
            card: DataCard = ...,
            columnName: str = "columnValues",
            rowName: str = "rowValues",
            ) -> DataCard:
        """
        Adds a table to a data card and stores it in the catalog.

        Parameters
        ----------
        indexVector : Union[FloatVectorIndex, Iterable[float]]
            The index vector for the card.
        tag : str
            The tag for the table.
        columnValues : Iterable[float]
            The column values for the table.
        rowValues : Iterable[float]
            The row values for the table.
        data : Iterable[Iterable[float]]
            The 2D data array for the table.
        card : DataCard, optional
            The data card to add the table to. If not provided, a new card is created.
        columnName : str, optional
            The name of the column values. Default is "columnValues".
        rowName : str, optional
            The name of the row values. Default is "rowValues".

        Returns
        -------
        DataCard
            The updated data card.

        Raises
        ------
        ValueError
            If the data cannot be converted to float64 or has invalid dimensions.
        TypeError
            If the tag is not a string.
        AssertionError
            If the dimensions of the data, columnValues, or rowValues are invalid.

        Examples
        --------
        >>> opat = OPAT()
        >>> opat.set_numIndex(2)
        >>> indexVector = [1.0, 2.0]
        >>> tag = "data"
        >>> columnValues = [1.0, 2.0, 3.0]
        >>> rowValues = [4.0, 5.0]
        >>> data = [[1.0, 2.0], [3.0, 4.0], [5.0, 6.0]]
        >>> opat.add_table(indexVector, tag, columnValues, rowValues, data)
        """
        try:
            cV = np.array(columnValues, dtype=np.float64)
            rV = np.array(rowValues, dtype=np.float64)
            d = np.array(data, dtype=np.float64)
        except ValueError as e:
            raise ValueError(f"Unable to convert data to float64. {e}")

        if not isinstance(tag, str):
            raise TypeError(f"tag must be a string! Currently it is {type(tag)}")
        
        assert cV.ndim == 1, f"columnValues must be a 1D array! Currently it has {cV.ndim} dimensions"
        assert rV.ndim == 1, f"rowValues must be a 1D array! Currently it has {rV.ndim} dimensions"
        assert d.ndim == 2, f"data must be a 2D array! Currently it has {d.ndim} dimensions"
        assert d.shape[1] == len(cV), f"data must have the same number of rows as columnValues! Currently it has {d.shape[0]} rows and {len(cV)} columns"
        assert d.shape[0] == len(rV), f"data must have the same number of columns as rowValues! Currently it has {d.shape[1]} columns and {len(rV)} rows"

        table = OPATTable(columnValues = cV, rowValues = rV, data = d)

        if card == ...:
            newCard = DataCard()
        else:
            newCard = card.copy()
        newCard.add_table(tag, table, columnName = columnName, rowName = rowName)

        self.add_card(indexVector, newCard)
        return newCard.copy()

    def __repr__(self) -> str:
        """
        Returns a string representation of the OPAT instance.

        Returns
        -------
        str
            The string representation of the OPAT instance.
        """
        reprString = f"""OPAT(
  version: {self.header.version}
  numCards: {self.header.numCards}
  headerSize: {self.header.headerSize}
  indexOffset: {self.header.indexOffset}
  creationDate: {self.header.creationDate}
  sourceInfo: {self.header.sourceInfo}
  comment: {self.header.comment}
  numIndex: {self.header.numIndex}
  hashPrecision: {self.header.hashPrecision}
  reserved: {self.header.reserved}
)"""
        return reprString

    def save_as_ascii(self, filename: str) -> str:
        """
        Saves the OPAT instance as a human-readable ASCII file.

        Parameters
        ----------
        filename : str
            The name of the file to save.

        Returns
        -------
        str
            The name of the saved file.
        """
        with open(filename, 'w') as f:
            f.write("This is an ASCII representation of an OPAT file, it is not a valid OPAT file in and of itself.\n")
            f.write("This file is meant to be human readable and is not meant to be read by a computer.\n")
            f.write("The purpose of this file is to provide a human readable representation of the OPAT file which can be used for debugging purposes.\n")
            f.write("The full binary specification of the OPAT file can be found in the OPAT file format documentation at:\n")
            f.write(" https://github.com/4D-STAR/4DSSE/blob/main/specs/OPAT/OPAT.pdf\n")
            f.write("="*35 + " HEADER " + "="*36 + "\n")
            f.write(f">> {self.header.magic}\n")
            f.write(f">> Version: {self.header.version}\n")
            f.write(f">> numCards: {self.header.numCards}\n")
            f.write(f">> headerSize (bytes): {self.header.headerSize}\n")
            f.write(f">> Card Catalog Offset (bytes): {self.header.catalogOffset}\n")
            f.write(f">> Creation Date: {self.header.creationDate}\n")
            f.write(f">> Source Info: {self.header.sourceInfo}\n")
            f.write(f">> Comment: {self.header.comment}\n")
            f.write(f">> numIndex: {self.header.numIndex}\n")
            f.write(f">> hashPrecision: {self.header.hashPrecision}\n")
            f.write("="*37 + " DATA " + "="*37 + "\n")
            f.write("="*80 + "\n")
            for card in self.cards.values():
                f.write(card.ascii())
            f.write("="*36 + " INDEX " + "="*37 + "\n")
            indexHeader = ""
            for indexID in range(self.header.numIndex):
                indexKey = f"Index {indexID}"
                indexHeader += f"{indexKey:<8} | "
            indexHeader += f"{'Byte Start':<15} {'Byte End':<15} {'Checksum (SHA-256)'}\n"
            f.write(indexHeader)
            f.write("="*80 + "\n")
            for indexID, index in self.catalog.items():
                f.write(index.ascii())

    def save(self, filename: str) -> str:
        """
        Saves the OPAT instance as a binary file.

        Parameters
        ----------
        filename : str
            The name of the file to save.

        Returns
        -------
        str
            The name of the saved file.

        Raises
        ------
        RuntimeError
            If the file cannot be saved.
        """
        with open(filename, 'wb') as f:
            f.write(bytes(self))
        if os.path.exists(filename):
            return filename
        else:
            raise RuntimeError(f"Unable to save file {filename}!")

    def __bytes__(self) -> bytes:
        """
        Converts the OPAT instance to bytes.

        Returns
        -------
        bytes
            The byte representation of the OPAT instance.
        """
        outBytes = b""
        outBytes += bytes(self.header)
        for card in self.cards.values():
            outBytes += bytes(card)
        for index in self.catalog.values():
            outBytes += bytes(index)
        return outBytes