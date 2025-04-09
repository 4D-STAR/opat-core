"""
The index subpackage for the opatio library.

This module defines a custom hashable multi-dimensional float index class
and provides a base class for handling OPAT file indices.

This index vector is hashed using xxhash with a specified precision to
avoid lookup issues based on floating-point precision. 

Modules
-------
floatvectorindex
    Defines the FloatVectorIndex class for handling hashable multi-dimensional
    float indices.

"""
from .floatvectorindex import FloatVectorIndex