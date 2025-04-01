"""
The load subpackage for the opatio library.

This module provides functions for loading data from OPAT files.
It makes the `read_opat` function available directly under the
'opatio.load' namespace.

Modules
-------
load
    Contains the implementation for reading OPAT files.

Examples
--------
>>> from opatio.load import read_opat
>>> data = read_opat("example.opat")
"""
from .load import read_opat