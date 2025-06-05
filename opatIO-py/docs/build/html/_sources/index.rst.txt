.. opatio documentation master file, created by
   sphinx-quickstart on Mon Mar 31 13:45:52 2025.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.
==================================
Welcome to opatio's documentation!
==================================

`opatio` is a Python library for handling OPAT (Open Parameterized Array Table) files...

Indroduction
============
OPAT is a structured binary file format defined by the 4D-STAR collaboration as the format
which will be used to store all tabular data for `SERiF`. This python module provides
tools for interacting with (reading, generating, and interpolating) OPAT files. In 
general the intended usage of `opatio` is that astronomers create opat tables 
for the physics they want to work with and then pass those files to `SERiF`. This
means that for the vast majority of people this module, the python module, is 
the vector through which they should interact with OPAT. 

If you are looking for information on the C++ library please reference `here <https://4d-star.github.io/opat-core/cpp/index.html>`_.

Getting Started
===============
If you are new to `opatio`, we reccoment you start with :ref:`Install` and :ref:`Usage`.

Help
====
If you have a question or find a bug please feel free to reach out to Emily M. Boudreaux (emily.boudreaux@dartmouth.edu) or submit an `issue <https://github.com/4D-STAR/opat-core/issues>`_ on the opat-core `github page <https://github.com/4D-STAR/opat-core>`_.

Table Of Contents
=================

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   installation
   usage_guide
   cpp

.. toctree::
   :maxdepth: 2
   :caption: API Reference:

   modules

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`


Funding
=======
opatio and OPAT-core are funded by the European Research Council (ERC) under
the Horizon Europe programme (Synergy Grant agreement No. 101071505: 4D-STAR).
Work for this review was partially funded by the Euro- pean Union. Views and
opinions expressed are however those of the author(s) only and do not
necessarily reflect those of the European Union or the European Research

