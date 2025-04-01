# opatIO python module
This module defines a set of tools to build, write, and read OPAT files. 
The OPAT fileformat is a custom file format designed to efficiently store
opacity information for a variety of compositions. 

## Installation
The prefered, stable, way to install opatio is to use the version on pypi
```bash
pip install opatio
```
If you wish to insall from source or for developemnt you can and should clone the git repository instead
You can install this module with pip
```bash
git clone https://github.com/4D-STAR/opat-core
cd opat-core/opatio-py
pip install . # For non development
```
or
```bash
pip install . -e # for development
```

## General Usage
The general way that this module is mean to be used is to first build a schema for the opacity table and then save that to disk. The module will handle all the byte aligment and lookup table construction for you. 

A simple example might look like the following

```python
from opatio import OPAT

opacityFile = OPAT()
opacityFile.set_comment("This is a sample opacity file")
opacityFile.set_source("OPLIB")

# some code to get a logR, logT, and logKappa table
# where logKappa is of size (n,m) if logR is size n and
# logT is size m

card = opacityFile.add_table((X, Z), "data", logR, logT, logKappa, rowName="logR", columnName="logT")
opacityFile.save("opacity.opat")
opacityFile.save_as_ascii("opacity.txt")
```

If you wish you can add more tables to the same card. This means that there can be multiple tables associated to the same indexing vector (for example you might store data and pre calculated interpolation coefficients along side the same data)

```python
opacityFile.add_table((X, Z), "interp", xOrder, yOrder, coeff, rowName="xCoeff", columnName="yCoeff", card=card)
```

Note how here I passed the card as an argument. This means that the same card will be modified and then repushed into the file.

You can also read opat files which have been generated with the loadOpat function

```python
from opatio import read_opat

opacityFile = read_opat("opacity.opat")

print(opacityFile.header)
```

## Converting from OPAL type I
Given the prevelence of OPAL type I tables, we have included a utility function to take care of this conversion for you. Assuming there is some OPAL type I file in your current working directory (in the below example we assume it is called `GS98hz`) then converting is as simple as calling the convert function...

```python
from opatio.convert import OPALI_2_OPAT

OPALI_2_OPAT("GS98hz", "gs98hz.opat")
```

## Problems
If you have problems feel free to either submit an issue to the root github repo (tagged as utils/opatio) or email Emily Boudreaux at emily.boudreaux@dartmouth.edu
