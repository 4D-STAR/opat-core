class OPATEntitity:
    """
    Represents a generic OPAT entity. This class serves as a base class for entities
    that need to define their size in bytes and provide a byte representation.

    Subclasses must implement the `__bytes__` method to define their specific
    byte representation.

    Examples
    --------
    >>> class MyEntity(OPATEntitity):
    ...     def __bytes__(self):
    ...         return b"example"
    ...
    >>> entity = MyEntity()
    >>> len(entity)
    7
    >>> bytes(entity)
    b'example'
    """

    def __len__(self) -> int:
        """
        Get the size of the entity in bytes.

        Returns
        -------
        int
            The size of the entity in bytes.

        Examples
        --------
        >>> class MyEntity(OPATEntitity):
        ...     def __bytes__(self):
        ...         return b"example"
        ...
        >>> entity = MyEntity()
        >>> len(entity)
        7
        """
        return len(bytes(self))
    
    def __bytes__(self) -> bytes:
        """
        Get the byte representation of the entity.

        Returns
        -------
        bytes
            The byte representation of the entity.

        Raises
        ------
        NotImplementedError
            If the method is not implemented in a subclass.

        Examples
        --------
        >>> class MyEntity(OPATEntitity):
        ...     def __bytes__(self):
        ...         return b"example"
        ...
        >>> entity = MyEntity()
        >>> bytes(entity)
        b'example'
        """
        raise NotImplementedError("Subclasses must implement __bytes__ method")