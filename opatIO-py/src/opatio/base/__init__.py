"""
The base subpackage for the opatio library.

This module makes the OPAT class available directly under the 'opatio.base'
namespace.

Modules
-------
opat
    Defines the main OPAT class for handling OPAT file data.
header
    Defines the Header class for OPAT file headers.

Examples
--------
>>> from opatio.base import OPAT
>>> obj = OPAT()
"""
from .opat import OPAT