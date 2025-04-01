import numpy as np
import re
import os

def get_OPAL_I_log_R() -> np.ndarray:
    """
    Generate the target log(R) values for OPAL type I files.

    Returns
    -------
    np.ndarray
        Array of log(R) values ranging from -8.0 to 1.0 with a step of 0.5.

    Examples
    --------
    >>> logR = get_OPAL_I_log_R()
    >>> print(logR)
    [-8.  -7.5 -7.  ...  0.5  1. ]
    """
    targetLogR = np.arange(-8.0, 1.5, 0.5)
    return targetLogR

def get_OPAL_I_log_T() -> np.ndarray:
    """
    Generate the target log(T) values for OPAL type I files.

    The spacing is non-uniform and is divided into three sections:
    - Section A: 3.75 to 6.0 with a step of 0.05
    - Section B: 6.0 to 8.1 with a step of 0.1
    - Section C: 8.1 to 8.8 with a step of 0.2

    Returns
    -------
    np.ndarray
        Array of log(T) values.

    Examples
    --------
    >>> logT = get_OPAL_I_log_T()
    >>> print(logT[:5])
    [3.75 3.8  3.85 3.9  3.95]
    """
    targetLogT_A = np.arange(3.75, 6.0, 0.05)
    targetLogT_B = np.arange(6.0, 8.1, 0.1)
    targetLogT_C = np.arange(8.1, 8.8, 0.2)
    targetLogT = np.concatenate((
        targetLogT_A,
        targetLogT_B,
        targetLogT_C
        ), axis=None)
    return targetLogT

def parse_OPAL_I(path: str) -> np.ndarray:
    """
    Parse OPAL type I data from a file.

    Parameters
    ----------
    path : str
        Path to the OPAL I file.

    Returns
    -------
    tuple
        A tuple containing:
        - logR (np.ndarray): Array of log(R) values.
        - logT (np.ndarray): Array of log(T) values.
        - p (np.ndarray): Parsed intensity data.

    Raises
    ------
    ValueError
        If the file does not contain the expected table structure.

    Examples
    --------
    >>> logR, logT, p = parse_OPAL_I("path/to/opal_file.txt")
    >>> print(logR.shape, logT.shape, p.shape)
    (19,) (81,) (n_tables, 81, 19)
    """
    with open(path) as f:
        contents = f.read().split('\n')
    sIndex = contents.index('************************************ Tables ************************************')
    ident = re.compile(r"TABLE\s+#(?:\s+)?\d+\s+\d+\s+X=\d\.\d+\s+Y=\d\.\d+\s+Z=\d\.\d+(?:\s+)?dX1=\d\.\d+\s+dX2=\d\.\d+")
    I = filter(lambda x: bool(re.match(ident, x[1])) and x[0] > sIndex+1, enumerate(contents))
    I = list(I)
    parsedTables = list(map(lambda x: [[float(z) for z in y.split()[1:]] for y in x], map(lambda x: contents[x[0]+6:x[0]+76], I)))

    paddedParsed = [list(map(lambda x: np.pad(x, (0, 19-len(x)), mode='constant', constant_values=(1,np.nan)), j)) for j in parsedTables]
    p = np.array(paddedParsed)

    return get_OPAL_I_log_R(), get_OPAL_I_log_T(), p

def OPALI_2_OPAT(inPath: str, outPath: str, saveAsASCII: bool = False) -> None:
    """
    Convert OPAL type I files to OPAT format.

    Parameters
    ----------
    inPath : str
        Path to the input OPAL type I file.
    outPath : str
        Path to save the converted OPAT file.
    saveAsASCII : bool, optional
        If True, also save the OPAT file as an ASCII file (default is False).

    Returns
    -------
    None

    Examples
    --------
    >>> OPALI_2_OPAT("path/to/opal_file.txt", "path/to/output.opat")
    >>> OPALI_2_OPAT("path/to/opal_file.txt", "path/to/output.opat", saveAsASCII=True)
    """
    compFind = re.compile(r"TABLE #\s*(\d+)\s+\d+\s+X=(\d\.\d+)\s+Y=(\d\.\d+)\s+Z=(\d\.\d+).+\n")
    with open(inPath, 'r') as f:
        contents = f.read()
    compMap = dict()
    matches = re.finditer(compFind, contents)
    for match in matches:
        e = match.groups()
        compMap[int(e[0]) - 1] = (float(e[1]), float(e[2]), float(e[3]))
    logR, logT, I = parse_OPAL_I(inPath)
    # Create the OPAT object
    from opatio import OPAT
    opat = OPAT()
    opat.set_comment(f"Converted from OPAL I {inPath}")
    opat.set_source(inPath)

    for table, (tabID, (X, Y, Z)) in zip(I, compMap.items()):
        tab2Add = table.T.copy()
        tab2Add[tab2Add == 9.999] = np.nan
        opat.add_table(
            (X, Z),
            "data",
            logT,
            logR,
            tab2Add,
            columnName="logR",
            rowName="logT",
        )

    # Save the OPAT file
    opat.save(outPath)

    if saveAsASCII:
        opat.save_as_ascii(os.path.splitext(outPath)[0] + ".dat")


