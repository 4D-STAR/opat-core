# OpatIO Core Libraries
This repository contains the core C++ library, python module, and file specification for the OPAT file format. 

OPAT is a structured binary file format developed by the 4D-STAR collaboration for efficient and standardized storage of opacity tables. 

The general principle behind OPAT is that an arbitrary number of opacity tables are stored, indexed by some arbitrary-length composition vector.

The provided python module can be used to create and read OPAT files while the C++ module is intended to be used to interface OPAT tables with C++ code for reading (but not currently generation).

## Installation
This repository provides both C++ and python bindings. The first thing to note is that these do not depend on each other at all. If you want to generate OPAT tables and/or use them in python code you will want the python module. If you want to use OPAT tables in C++ code you will want the C++ module. 

There are more details on usage for each language in their respective folder; however, broad installation instructions are included here as well.

### Python Installation
```bash
pip install opatio
```

### C++ Installation
You will need `meson`, `cmake`, and `ninja` installed pre-installed. These can be installed with pip
```bash
pip install meson>=1.6.0
pip install cmake
pip install ninja
```
Then you can build and install opat-core
```bash
git clone https://github.com/4D-STAR/opat-core
cd opat-core
meson setup build --buildtype=release
meson compile -C build
```
If you want to run tests
```bash
meson test -C build
```
To install headers, libraries, and the command line utilities
```
meson install -C build
```
# Usage

## Python Usage
See opatIO-py for installation instructions. A sample python program which can be used to generate OPAT tables fom Type 1 OPAL tables is included below (This example was written by Aaron Dotter)

```python 
from pylab import *
from opatio import OpatIO

input_filename="GS98hz"

output_filename="GS98hz.opat"

number_of_tables=126
lines_per_table=77

T1=[3.75 + 0.05*i for i in range(46)]
T2=[6.1+0.1*i for i in range(21)]
T3=[8.3,8.5,8.7]
logT=T1+T2+T3
NUM_T=len(logT)

logR=[-8.0+0.5*i for i in range(19)]
NUM_R=len(logR)

def process_comp_info(line):
    Tnum =    int(line[7:10])
    Xval = float(line[36:42])
    Yval = float(line[45:51])
    Zval = float(line[54:60])
    return Tnum,Xval,Zval

def table_data(line):
    values=line.rstrip().split()
    data=-9.999*ones(NUM_R)
    for i in range(1,len(values)):
        data[i-1] = float(values[i])
    return data

with open("GS98hz","r") as f:
    contents=f.readlines()

o = OpatIO()
o.set_comment("OPAL Type I table with Grevesse & Sauval (1998) composition")
o.set_source("OPAL file GS98hz")
    
#first 240 lines are header
skip=239 #lines that says "***** Tables *****"
for j in range(number_of_tables):
    comp_index = skip + 2 + lines_per_table*j

    Tnum,Xval,Zval=process_comp_info(contents[comp_index].rstrip())

    table_index=comp_index+6
    logkappa=zeros((NUM_R,NUM_T))
    for i in range(NUM_T):
        result=table_data(contents[table_index+i])
        for k in range(len(result)):
            logkappa[k,i] = result[k]
    o.add_table((Xval, Zval), logR, logT, logkappa)

o.save("GS98hz.opat")
o.save_as_ascii("GS98hz_OPAT.ascii")
```

## C++ Library/Header Usage
See OpatIO-cpp for cpp building and installation instructions. General use in c++ might look like

```cpp
#import "opatIO.h"

int main() {
    OpatIO opat("GS98hz.opat");
    OPATable table = opat.getTable({0.1, 0.001});
}
```

## Command Line Utility Usage
If you run `meson install` there will be three command line utilities avalible in your path (you may have to resource your shell either by closing and reopening the terminal or by typing `source ~/.zshrc` if on zsh or `source ~/.bashrc` if on bash. These utilities are `opatHeader`, `opatInspect`, and `opatVerify`. They: print out the header, print the header and the table index, print if the provided file is a valid OPAT file respectivley. Usgae of these tools looks like

```bash
opatHeader -f path/to/file.opat
```

Note the `-f`, that is needed before providing the path to the file. Usage for all tools looks the same.
