import struct
from dataclasses import dataclass
from datetime import datetime

from opatio.misc.opatentity import OPATEntitity


@dataclass
class Header(OPATEntitity):
    """
    A class to represent the header information of an OPAT file.

    Attributes
    ----------
    version : int
        Version of the OPAT file format.
    numCards : int
        Number of tables in the file.
    headerSize : int
        Size of the header in bytes.
    catalogOffset : int
        Offset to the index section.
    creationDate : str
        Creation date of the file.
    sourceInfo : str
        Source information of the file.
    comment : str
        Comment section of the file.
    numIndex : int
        Number of values to use when indexing the table.
    hashPrecision : int
        Precision of the hash.
    reserved : bytes
        Reserved for future use (default is 23 null bytes).
    magic : str
        Magic number to identify the file format (default is "OPAT").

    Methods
    -------
    set_comment(comment: str)
        Sets the comment of the header.
    set_source(source: str)
        Sets the source information of the header.
    __bytes__()
        Converts the header to bytes.
    __repr__()
        Returns the string representation of the header.
    """

    version: int
    numCards: int
    headerSize: int
    catalogOffset: int
    creationDate: str
    sourceInfo: str
    comment: str
    numIndex: int
    hashPrecision: int
    reserved: bytes = b"\x00"*23
    magic: str = "OPAT"

    def set_comment(self, comment: str):
        """
        Sets the comment of the header.

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
        >>> header = Header(...)
        >>> header.set_comment("This is a comment.")
        """
        if len(comment) >= 128:
            raise ValueError(
                f"comment string ({comment}) is too long ({len(comment)}). Max length is 128"
            )
        self.comment = comment

    def set_source(self, source: str):
        """
        Sets the source information of the header.

        Parameters
        ----------
        source : str
            The source information to set.

        Raises
        ------
        ValueError
            If the source string exceeds 64 characters.

        Examples
        --------
        >>> header = Header(...)
        >>> header.set_source("Source information")
        """
        if len(source) >= 64:
            raise ValueError(
                f"sourceInfo string ({source}) is too long ({len(source)}). Max length is 64"
            )
        self.sourceInfo = source

    def __bytes__(self) -> bytes:
        """
        Converts the header to bytes.

        Returns
        -------
        bytes
            The header as a byte sequence.

        Raises
        ------
        AssertionError
            If the resulting byte sequence is not 256 bytes long.

        Examples
        --------
        >>> header = Header(...)
        >>> header_bytes = bytes(header)
        """
        headerBytes = struct.pack(
            "<4s H I I Q 16s 64s 128s H B 23s",
            self.magic.encode('utf-8'),
            self.version,
            self.numCards,
            self.headerSize,
            self.catalogOffset,
            self.creationDate.encode('utf-8'),
            self.sourceInfo.encode('utf-8'),
            self.comment.encode('utf-8'),
            self.numIndex,
            self.hashPrecision,
            self.reserved
        )
        assert len(headerBytes) == 256, (
            f"Header must be 256 bytes. Due to an unknown error the header has {len(headerBytes)} bytes"
        )
        return headerBytes

    def __repr__(self) -> str:
        """
        Returns the string representation of the header.

        Returns
        -------
        str
            The string representation of the header.

        Examples
        --------
        >>> header = Header(...)
        >>> print(header)
        """
        return (
            f"Header(magic={self.magic}, version={self.version}, numCards={self.numCards}, "
            f"headerSize={self.headerSize}, catalogOffset={self.catalogOffset}, "
            f"creationDate={self.creationDate}, sourceInfo={self.sourceInfo}, "
            f"comment={self.comment}, numIndex={self.numIndex})"
        )


def make_default_header() -> Header:
    """
    Creates a default header for an OPAT file.

    Returns
    -------
    Header
        The default header.

    Examples
    --------
    >>> default_header = make_default_header()
    >>> print(default_header)
    """
    return Header(
        version=1,
        numCards=0,
        headerSize=256,
        catalogOffset=0,
        creationDate=datetime.now().strftime("%b %d, %Y"),
        sourceInfo="no source provided by user",
        comment="default header",
        numIndex=2,
        hashPrecision=8,
    )