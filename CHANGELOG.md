## 0.3.1a (2025-06-10)

### Feat

- **opatIO-JS**: added JS code for opat parsing
- **docs**: added interactive browser opat-viewer

### Refactor

- **DataCard::get**: added more informative error messages

## 0.3.0a (2025-06-05)

### BREAKING CHANGE

- boost and qhull are now needed to install

### Feat

- **opatIO-py**: added TableLattice to python module
- **opatIO-cpp**: small changed including adding size utilty functions to many classes
- **tableLattice**: added table lattice interpolation
- **opatio**: added method to convert __init__
- **opatio-cpp**: updated opatio-cpp to allow for vector cell elements
- **opatio**: added ability to store vectors in each cell
- **opatio/index**: added FloatIndexVector to index __init__
- **opatIO-fortran**: implimented fortran module
- **opatio-cpp**: added detailed example code
- **opatio-cpp**: added overload for () slicing
- **opat**: opat supports data cards
- **opat-spec**: updated opat spec to use arbitrary data cards
- **spec**: added latex and make
- **opat.magic**: added opat magic for file utility
- **.gitignore**: addded gitignore
- **opatIO-py**: added first opatIO-py commit
- **opatIO-cpp**: first commit of opatIO-cpp to new opat repo. For full history see https://github.com/4D-STAR/4DSSE pre commit 001ddbf055

### Fix

- **tableLattice**: switched to walk algorithm
- **opat-core**: fixed memcpy and stream insertion operator when compiling with gcc 13.3.0
- **OPATTable**: __bytes__ flipped the order of rV and cV before. This has been resolved
- **OPAL1_2_OPAT**: correct row/column order
- **opatio**: fixed bug where num tables = 2* true value
- **opatio**: fixed minor bugs
- **opatio**: opatio finds modules and fixed typo
- **cache**: deleted package cache from git
- **.gitignore**: added googletest and cache
- **tests**: fixed path and subprojects directory to allow for building and running tests as a subproject correctly
- **opatIO-cpp**: added subprojects to git ignore
- **opatIO-cpp**: added subprojects to git ignore
- **opatIO-cpp**: moved around meson.build to make work with wrap
- **readme**: fixed some typos in readme
- **readme**: removed duplicated lines in python example

### Refactor

- **tests**: ran tests
- **gitignore**: added .idea to gitignore
