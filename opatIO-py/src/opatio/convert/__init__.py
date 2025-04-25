"""
The convert subpackage for the opatio library.

This module provides functions for converting other file formats
to the OPAT format. It currently makes the `OPALI_2_OPAT` function
available for converting OPAL Type I files.

Modules
-------
opal
    Contains functions for converting OPAL data formats.

Examples
--------
>>> from opatio.convert import OPALI_2_OPAT
>>> # Convert an OPAL Type I file to OPAT format
>>> OPALI_2_OPAT("GS98hz", "GS98hz.opat")

>>> # Load the converted OPAT file
>>> from opatio import read_opat
>>> opat = read_opat("GS98hz.opat")

Or you can use the `load_opat1_as_opat` function to load OPAL Type I directly
>>> from opatio.convert import load_opat1_as_opat
>>> opat = load_opat1_as_opat("GS98hz")
"""
from .opal import OPALI_2_OPAT
from .opal import load_opat1_as_opat