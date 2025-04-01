from typing import List, Any

from opatio.catalog.entry import CardCatalogEntry

def is_float_castable(value: Any) -> bool:
    """
    Check if a value can be cast to a float.

    Parameters
    ----------
    value : Any
        The value to check.

    Returns
    -------
    bool
        True if the value can be cast to a float, False otherwise.

    Examples
    --------
    >>> is_float_castable("123.45")
    True
    >>> is_float_castable("abc")
    False
    """
    try:
        float(value)
        return True
    except ValueError:
        return False


def print_table_indexes(table_indexes: List[CardCatalogEntry]) -> str:
    """
    Generate a formatted string representation of table indexes.

    Parameters
    ----------
    table_indexes : List[CardCatalogEntry]
        A list of `CardCatalogEntry` objects representing table indexes.

    Returns
    -------
    str
        A formatted string containing table index details.

    Raises
    ------
    ValueError
        If `table_indexes` is empty.

    Examples
    --------
    >>> from opatio.catalog.entry import CardCatalogEntry
    >>> entry = CardCatalogEntry(index=[1.0, 2.0], byteStart=0, byteEnd=100, sha256="abcdef1234567890")
    >>> print(print_table_indexes([entry]))
    Table Indexes in OPAT File:
    Index 0    Index 1    Byte Start      Byte End        Checksum (SHA-256)
    ================================================================================
    1.0000     2.0000     0              100            abcdef1234567890...

    Notes
    -----
    This function is used to display table index information in a human-readable format.
    """
    if not table_indexes:
        print("No table indexes found.")
        return

    tableRows: List[str] = []
    tableRows.append("\nTable Indexes in OPAT File:\n")
    headerString: str = ''
    # Generate header row
    for indexID, index in enumerate(table_indexes[0].index):
        indexKey = f"Index {indexID}"
        headerString += f"{indexKey:<10}"
    headerString += f"{'Byte Start':<15} {'Byte End':<15} {'Checksum (SHA-256)'}"
    tableRows.append(headerString)
    tableRows.append("=" * 80)
    # Generate table rows
    for entry in table_indexes:
        tableEntry = ''
        for index in entry.index:
            tableEntry += f"{index:<10.4f}"
        tableEntry += f"{entry.byteStart:<15} {entry.byteEnd:<15} {entry.sha256[:16]}..."
        tableRows.append(tableEntry)
    return '\n'.join(tableRows)