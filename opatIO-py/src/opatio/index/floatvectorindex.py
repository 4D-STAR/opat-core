from dataclasses import dataclass
from typing import Tuple
import struct
import xxhash

@dataclass
class FloatVectorIndex:
    """
    Represents an index for a float vector with hashing and serialization capabilities.

    Parameters
    ----------
    vector : Tuple[float]
        The tuple of floats representing the vector.
    hashPrecision : int
        The precision to which floats are rounded for hashing.

    Methods
    -------
    __hash__() -> int
        Compute the hash of the float vector index.
    __bytes__() -> bytes
        Convert the float vector index to bytes.
    __len__() -> int
        Get the number of elements in the float vector.
    __repr__() -> str
        Get the string representation of the float vector index.
    __getitem__(index: int) -> float
        Get the item from vector at the specified index.
    copy() -> FloatVectorIndex
        Create a copy of the float vector index.

    Examples
    --------
    >>> index = FloatVectorIndex((1.12345, 2.6789), hashPrecision=3)
    >>> hash(index)
    1234567890123456789
    >>> bytes(index)
    b'...'
    >>> len(index)
    2
    >>> index[0]
    1.123
    """
    vector: Tuple[float, ...]
    hashPrecision: int

    def __hash__(self) -> int:
        """
        Compute the hash of the float vector index.

        Returns
        -------
        int
            The hash of the float vector index.

        Notes
        -----
        The hash is computed by rounding the floats in the vector to the specified
        precision, packing them into a byte array, and then hashing the byte array
        using xxhash.

        Examples
        --------
        >>> index = FloatVectorIndex((1.12345, 2.6789), hashPrecision=3)
        >>> hash(index)
        1234567890123456789
        """
        rounded_vector = tuple(round(v, self.hashPrecision) for v in self.vector)
        floatByteArray = struct.pack(f"<{len(self.vector)}d", *rounded_vector)
        return xxhash.xxh64(floatByteArray).intdigest()

    def __bytes__(self) -> bytes:
        """
        Convert the float vector index to bytes.

        Returns
        -------
        bytes
            The float vector index as a byte array.

        Notes
        -----
        The floats in the vector are rounded to the specified precision before
        being packed into a byte array.

        Examples
        --------
        >>> index = FloatVectorIndex((1.12345, 2.6789), hashPrecision=3)
        >>> bytes(index)
        b'...'
        """
        rounded_vector = tuple(round(v, self.hashPrecision) for v in self.vector)
        return struct.pack(f"<{len(self.vector)}d", *rounded_vector)

    def __len__(self) -> int:
        """
        Get the number of elements in the float vector.

        Returns
        -------
        int
            The number of elements in the vector.

        Examples
        --------
        >>> index = FloatVectorIndex((1.12345, 2.6789), hashPrecision=3)
        >>> len(index)
        2
        """
        return len(self.vector)

    def __repr__(self) -> str:
        """
        Get the string representation of the float vector index.

        Returns
        -------
        str
            The string representation of the float vector index.

        Examples
        --------
        >>> index = FloatVectorIndex((1.12345, 2.6789), hashPrecision=3)
        >>> repr(index)
        'FloatVectorIndex((1.12345, 2.6789))'
        """
        return f"FloatVectorIndex({self.vector})"
    
    def __getitem__(self, index: int) -> float:
        """
        Get the item from the vector at the specified index.

        Parameters
        ----------
        index : int
            The index of the item to retrieve.

        Returns
        -------
        float
            The item at the specified index.

        Raises
        ------
        IndexError
            If the index is out of range.

        Examples
        --------
        >>> index = FloatVectorIndex((1.12345, 2.6789), hashPrecision=3)
        >>> index[0]
        1.12345
        """
        return self.vector[index]

    def copy(self) -> 'FloatVectorIndex':
        """
        Create a copy of the float vector index.

        Returns
        -------
        FloatVectorIndex
            A new instance of FloatVectorIndex with the same values.

        Examples
        --------
        >>> index = FloatVectorIndex((1.12345, 2.6789), hashPrecision=3)
        >>> index_copy = index.copy()
        >>> index_copy.vector
        (1.12345, 2.6789)
        """
        return FloatVectorIndex(vector=self.vector, hashPrecision=self.hashPrecision)