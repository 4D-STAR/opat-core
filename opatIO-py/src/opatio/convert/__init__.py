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
"""
from .opal import OPALI_2_OPAT