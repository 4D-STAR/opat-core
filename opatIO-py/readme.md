# opatIO python module
This module defines a set of tools to build, write, and read OPAT files. 
The OPAT fileformat is a custom file format designed to efficiently store
opacity information for a variety of compositions. 

## Installation
You can install this module with pip
```bash
git clone <repo>
cd 4DSSE/utils/opat
pip install .
```

## General Usage
The general way that this module is mean to be used is to first build a schema for the opacity table and then save that to disk. The module will handle all the byte aligment and lookup table construction for you. 

A simple example might look like the following

```python
from opatio import OpatIO

opacityFile = OpatIO()
opacityFile.set_comment("This is a sample opacity file")
opaticyFile.set_source("OPLIB")

# some code to get a logR, logT, and logKappa table
# where logKappa is of size (n,m) if logR is size n and
# logT is size m

opacityFile.add_table((X, Z), logR, logT, logKappa)
opacityFile.save("opacity.opat")
opaticyFile.save_as_ascii("opacity.txt")
```

You can also read opat files which have been generated with the loadOpat function

```python
from opatio import loadOpat

opacityFile = loadOpat("opacity.opat")

print(opacityFile.header)
print(opaticyFile.tables[0])
```

## Problems
If you have problems feel free to either submit an issue to the root github repo (tagged as utils/opatio) or email Emily Boudreaux at emily.boudreaux@dartmouth.edu