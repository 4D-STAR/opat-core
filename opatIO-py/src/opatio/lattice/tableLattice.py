from opatio.base.opat import OPAT
from opatio.card.datacard import OPATTable
from opatio.card.datacard import DataCard
from opatio.index.floatvectorindex import FloatVectorIndex

import numpy as np

from scipy.spatial import Delaunay

from typing import Tuple, List

class TableLattice:
    """
    A class for performing interpolation over a lattice of data points using Delaunay triangulation.

    This class is useful for interpolating data stored in an OPAT object, which contains index vectors
    and associated data cards. It builds a Delaunay triangulation of the index vectors and allows
    querying for interpolated data cards based on a given query vector.

    Parameters
    ----------
    opat : OPAT
        The OPAT object containing index vectors and associated data cards.

    Attributes
    ----------
    opat : OPAT
        The OPAT object passed during initialization.
    opatSize : Tuple[int, int]
        The size of the OPAT object (dimensions of the data).
    indexVectors : List[FloatVectorIndex]
        The list of index vectors from the OPAT object.
    triangulation : scipy.spatial.Delaunay
        The Delaunay triangulation built from the index vectors.

    Examples
    --------
    >>> from opatio.base.opat import OPAT
    >>> from opatio.lattice.tableLattice import TableLattice
    >>> opat = OPAT(...)  # Initialize OPAT object
    >>> lattice = TableLattice(opat)
    >>> query_vector = FloatVectorIndex(...)  # Create a query vector
    >>> result_card = lattice.get(query_vector)
    >>> print(result_card)
    """

    def __init__(self, opat: OPAT):
        """
        Initialize the TableLattice object.

        This method sets up the OPAT object, retrieves its size and index vectors, and builds
        the Delaunay triangulation.

        Parameters
        ----------
        opat : OPAT
            The OPAT object containing index vectors and associated data cards.
        """
        self.opat : OPAT = opat

        self.opatSize: Tuple[int, int] = opat.size()
        self.indexVectors: List[FloatVectorIndex] = opat.indexVectors
        self._build_delaunay()

    def get(self, query: FloatVectorIndex) -> DataCard:
        """
        Interpolate a data card based on a query vector using barycentric weights.

        This method finds the simplex containing the query vector, calculates barycentric weights,
        and interpolates the associated data cards of the simplex vertices to produce a result data card.

        Parameters
        ----------
        query : FloatVectorIndex
            The index vector to query for interpolation.

        Returns
        -------
        DataCard
            The interpolated data card.

        Raises
        ------
        ValueError
            If the query point is not contained in any simplex.
        IndexError
            If a vertex ID is out of bounds for the index vectors.

        Examples
        --------
        >>> query_vector = FloatVectorIndex(...)  # Create a query vector
        >>> result_card = lattice.get(query_vector)
        >>> print(result_card)
        """
        simplexID: int = self._find_containing_simplex(query)
        if simplexID is None:
            raise ValueError("Query point is not contained in any simplex.")
        vertexIDs = self.triangulation.simplices[simplexID]
        verticies: List[DataCard] = []
        for vertexID in vertexIDs:
            if vertexID < 0 or vertexID >= len(self.indexVectors):
                raise IndexError(f"Vertex ID {vertexID} is out of bounds for index vectors.")
            verticies.append(self.opat[self.indexVectors[vertexID].vector])

        tagOkay: bool = True
        # Check if all DataCards have the same tags
        tags = set(verticies[0].keys())
        for DC in verticies:
            if set(DC.keys()) != tags:
                tagOkay = False
                break

        dim: int = self.opatSize[0]
        transform: np.ndarray = self.triangulation.transform[simplexID]
        offset: np.ndarray = transform[dim]
        matrix_d: np.ndarray = transform[:dim]

        delta: np.ndarray = query.vector - offset
        bary_partial: np.ndarray = matrix_d.dot(delta)
        last_weight: float = 1.0 - bary_partial.sum()

        weights: np.ndarray = np.concatenate([bary_partial, np.array([last_weight])])  # shape (dim+1,)

        resultCard: DataCard = DataCard()
        resultCard.header = verticies[0].header  # copy header from the first vertex

        for tag in tags:
            #get column names and row names from the datacard's index
            columnName: str = verticies[0].index[tag].columnName
            rowName: str = verticies[0].index[tag].rowName
            # Start a fresh OPATTable for this tag
            # Copy columnValues and rowValues from the first vertex’s table
            firstTable = verticies[0][tag]

            # Allocate zero‐array matching the shape of firstTable.data
            accumulator: np.ndarray = np.zeros_like(firstTable.data)

            # Accumulate weighted sum over all vertices
            for w, DC in zip(weights, verticies):
                srcTable: OPATTable = DC[tag]
                accumulator += w * srcTable.data

            # Attach the interpolated table under this tag
            resultTable: OPATTable = OPATTable(firstTable.columnValues, firstTable.rowValues, accumulator)
            resultCard.add_table(tag, resultTable, columnName=columnName, rowName=rowName)

        return resultCard


    def _build_delaunay(self):
        """
        Build the Delaunay triangulation for the index vectors.

        This method constructs a Delaunay triangulation using the index vectors from the OPAT object.
        It ensures that the triangulation is valid and raises an error if the points are collinear.

        Raises
        ------
        ValueError
            If the Delaunay triangulation fails due to invalid or collinear points.

        Examples
        --------
        >>> lattice._build_delaunay()
        """
        points : np.ndarray = np.zeros(shape=(self.opatSize[1], self.opatSize[0]), dtype=np.float64)
        for i, iv in enumerate(self.indexVectors):
            points[i] = iv.vector
        try:
            self.triangulation = Delaunay(points)
        except ValueError as e:
            raise ValueError("Failed to create Delaunay triangulation. Ensure that the index vectors are valid and not collinear.") from e

    def _find_containing_simplex(self, query: FloatVectorIndex) -> int:
        """
        Find the index of the simplex that contains the query point.

        This method uses the Delaunay triangulation to locate the simplex containing the query vector.

        Parameters
        ----------
        query : FloatVectorIndex
            The index vector to query.

        Returns
        -------
        int
            The index of the containing simplex, or None if the query point is not contained in any simplex.

        Examples
        --------
        >>> simplex_index = lattice._find_containing_simplex(query_vector)
        >>> print(simplex_index)
        """
        simplex_index = self.triangulation.find_simplex(query.vector)
        return simplex_index if simplex_index >= 0 else None
