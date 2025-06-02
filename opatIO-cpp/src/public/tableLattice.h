#pragma once

#include <memory>
#include <vector>
#include <utility>

#include "opatIO.h"
#include "indexVector.h"

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

/**
 * @brief Namespace for table lattice interpolation of OPAT files.
 */
namespace opat::lattice {

    /**
     * @brief Typedef for a Boost uBLAS matrix of doubles.
     */
    typedef boost::numeric::ublas::matrix<double> bmat;
    /**
     * @brief Typedef for a Boost uBLAS vector of doubles.
     */
    typedef boost::numeric::ublas::vector<double> bvec;

    /**
     * @brief Represents a simplex containing a point, along with barycentric weights.
     *
     * **Example:**
     * @code
     * opat::lattice::Simplex simplex_info;
     * simplex_info.ID = 5; // Simplex ID from the triangulation
     * simplex_info.barycentricWeights = {0.25, 0.5, 0.25}; // Weights for a 2D simplex (triangle)
     * // An ID of static_cast<std::size_t>(-1) indicates an invalid or not-found simplex.
     * @endcode
     */
    struct Simplex {
        std::size_t ID = static_cast<std::size_t>(-1); ///< The ID of the simplex in the triangulation. Defaults to an invalid ID.
        std::vector<double> barycentricWeights; ///< Barycentric weights of the point within this simplex.
    };

    /**
     * @brief Defines the type of interpolation to be used.
     */
    enum class InterpolationType {
        Linear,    ///< Linear interpolation.
        Quadratic, ///< Quadratic interpolation (Not yet implemented).
        Cubic      ///< Cubic interpolation (Not yet implemented).
    };

    /**
     * @brief Represents a lattice structure for interpolating data from an OPAT object.
     *
     * The TableLattice class builds a Delaunay triangulation of the OPAT index points
     * and provides methods to interpolate data for a given index vector.
     *
     * **Example:**
     * @code
     * #include "opatIO.h"       // For opat::readOPAT, opat::OPAT, opat::DataCard
     * #include "indexVector.h"  // For FloatIndexVector
     * #include "tableLattice.h" // For opat::lattice::TableLattice
     * #include <iostream>
     * #include <string>
     * #include <vector>
     *
     * int main() {
     *     // Define the path to your OPAT file
     *     std::string opat_filename = "gs98Hz.opat"; // Replace with your actual file path
     *
     *     // 1. Load the OPAT file
     *     opat::OPAT opat_data = opat::readOPAT(opat_filename);
     *     std::cout << "Successfully loaded OPAT file: " << opat_filename << std::endl;
     *
     *     // 2. Create a TableLattice object from the loaded OPAT data
     *     // This will automatically build the Delaunay triangulation.
     *     // Default interpolation is Linear. (Also the only one currently implemented)
     *     opat::lattice::TableLattice lattice(opat_data);
     *     std::cout << "TableLattice initialized." << std::endl;
     *
     *     // Optionally, set a different interpolation type (not yet implemented)
     *     // lattice.setInterpolationType(opat::lattice::InterpolationType::Quadratic);
     *
     *     // 3. Define an index vector for which you want to interpolate data.
     *     // The dimension of this vector must match opat_data.header.numIndex.
     *     // For this example, let's assume a 2D OPAT file, and we want data for (0.54421, 0.077585).
     *     FloatIndexVector query_point({0.54421, 0.077585});
     *     std::cout << "Querying for index vector: " << query_point << std::endl;
     *
     *     // 4. Get the interpolated DataCard for the query_point.
     *     opat::DataCard interpolated_card = lattice.get(query_point);
     *
     *     // 5. Access a specific table from the interpolated DataCard.
     *     std::string target_table_tag = "data";
     *
     *     const opat::OPATTable& interpolated_table = interpolated_card[target_table_tag];
     *     std::cout << "Successfully accessed interpolated table: " << target_table_tag << std::endl;
     *
     *     // You can now use the interpolated_table, e.g., print it or extract values
     *     // interpolated_table.print();
     *
     *     // Example of dumping the triangulation (useful for debugging)
     *     // lattice.dumpTriangulationToAscii("points_dump.txt", "simplices_dump.txt");
     *     // std::cout << "Triangulation dumped to files." << std::endl;
     *
     *     return 0;
     * }
     * @endcode
     */
    class TableLattice {
    public:
        /**
         * @brief Constructs a TableLattice object from an OPAT object.
         *
         * Initializes the lattice using the index vectors from the OPAT object
         * and builds a Delaunay triangulation using Qhull. Defaults to Linear interpolation.
         * @param opat The OPAT object containing the data.
         * @throws std::runtime_error if Delaunay triangulation construction fails (e.g., due to Qhull errors,
         *         which could be caused by insufficient or degenerate input points from the OPAT file).
         *         Resolution: Ensure the OPAT file contains valid and sufficient index points for triangulation.
         *
         * **Example:**
         * @code
         * opat::OPAT my_opat_data = opat::readOPAT("my_data.opat");
         * opat::lattice::TableLattice lattice(my_opat_data);
         * // Use the lattice...
         * @endcode
         */
        explicit TableLattice(const opat::OPAT& opat);
        /**
         * @brief Constructs a TableLattice object from an OPAT object with a specified interpolation type.
         *
         * Initializes the lattice using the index vectors from the OPAT object
         * and builds a Delaunay triangulation using Qhull.
         * @param opat The OPAT object containing the data.
         * @param interpolationType The type of interpolation to use.
         * @throws std::runtime_error if the specified `interpolationType` is not `InterpolationType::Linear`,
         *         as other types are not yet implemented.
         * @throws std::runtime_error if Delaunay triangulation construction fails (e.g., due to Qhull errors,
         *         which could be caused by insufficient or degenerate input points from the OPAT file).
         *         Resolution: Ensure the OPAT file contains valid and sufficient index points for triangulation.
         *                     Use `InterpolationType::Linear`.
         *
         * **Example:**
         * @code
         * opat::OPAT my_opat_data = opat::readOPAT("my_data.opat");
         * opat::lattice::TableLattice lattice(my_opat_data, opat::lattice::InterpolationType::Linear);
         * // Use the lattice
         * @endcode
         */
        TableLattice(const opat::OPAT& opat, const InterpolationType& interpolationType);

        TableLattice(const TableLattice&) = default;
        TableLattice& operator=(const TableLattice&) = delete;
        TableLattice(TableLattice&&) noexcept = default;
        TableLattice& operator=(TableLattice&&) noexcept = delete;
        ~TableLattice() = default;

        /**
         * @brief Retrieves interpolated data for a given index vector.
         *
         * Finds the containing simplex for the index vector and performs
         * barycentric interpolation of the data from the simplex vertices.
         * @param indexVector The index vector for which to interpolate data.
         * @return A DataCard containing the interpolated data.
         * @throws std::out_of_range if the `indexVector` is outside the bounds of the table data (as determined by `validateIndexVector`),
         *         or if `findContainingSimplex` determines the point is outside the convex hull of the data points,
         *         or if an internal ID (vertex/simplex) is out of bounds.
         *         Resolution: Ensure `indexVector` values are within the min/max range for each dimension (check `opat::OPAT::getBounds()`).
         *                     Ensure the point lies within the convex hull of the OPAT index points.
         * @throws std::invalid_argument if the `indexVector` has a dimension different from the OPAT data's index dimension
         *         (checked by `validateIndexVector` or `findContainingSimplex`).
         *         Resolution: Ensure `indexVector.size()` matches `opat_object.header.numIndex`.
         * @throws std::runtime_error if `findContainingSimplex` fails due to internal errors (e.g., no simplices, cycle in walk,
         *         cannot determine exit face, max walk steps reached), or if `calculateBarycentricWeights` fails due to a
         *         degenerate simplex (singular matrix).
         *         Resolution: These errors often indicate issues with the underlying triangulation or extreme numerical conditions.
         *                     Verify the integrity of the input OPAT file's index data.
         * @throws std::logic_error if `calculateBarycentricWeights` returns an unexpected number of weights (internal logic error).
         *
         * **Example:**
         * @code
         * // Assuming 'lattice' is an initialized TableLattice object
         * // and 'my_opat_object' is the OPAT object it was built from.
         * // For an OPAT with 2 index dimensions:
         * FloatIndexVector target_iv({0.25, 0.75});
         * opat::DataCard result = lattice.get(target_iv);
         * // Use 'result' which contains interpolated data
         * @endcode
         */
        [[nodiscard]] DataCard get(const FloatIndexVector& indexVector) const;
        /**
         * @brief Gets the current interpolation type.
         * @return The current InterpolationType.
         */
        [[nodiscard]] InterpolationType getInterpolationType() const;

        /**
         * @brief Sets the interpolation type.
         * @param interpolationType The new InterpolationType to set.
         * @throws std::runtime_error if `interpolationType` is not `InterpolationType::Linear`,
         *         as other types are not currently implemented.
         *         Resolution: Only use `InterpolationType::Linear`.
         *
         * **Example:**
         * @code
         * // Assuming 'lattice' is an initialized TableLattice object
         * lattice.setInterpolationType(opat::lattice::InterpolationType::Linear);
         * @endcode
         */
        void setInterpolationType(InterpolationType interpolationType);

        /**
         * @brief Dumps the Delaunay triangulation to ASCII files.
         *
         * Creates two files: one for the points (vertices) and one for the simplices.
         * This is useful for visualizing or debugging the triangulation.
         * @param points_file The path to the file where points will be saved.
         * @param simplices_file The path to the file where simplices will be saved.
         * @throws std::ios_base::failure if an error occurs during file I/O operations (e.g., cannot open file for writing).
         *         Resolution: Ensure the program has write permissions to the specified file paths and that the paths are valid.
         *
         * **Example:**
         * @code
         * // Assuming 'lattice' is an initialized TableLattice object
         * lattice.dumpTriangulationToAscii("my_points.txt", "my_simplices.txt");
         * // This will create two files:
         * // "my_points.txt" containing the coordinates of the triangulation vertices.
         * // "my_simplices.txt" containing the vertex indices for each simplex.
         * @endcode
         */
        void dumpTriangulationToAscii(const std::string &points_file, const std::string &simplices_file) const;
    private:
        const opat::OPAT &m_opat; ///< Reference to the OPAT object.
        std::size_t m_indexVectorSize{}; ///< The dimensionality of the index vectors.
        InterpolationType m_interpolationType{InterpolationType::Linear}; ///< The type of interpolation to use.
        std::vector<FloatIndexVector> m_indexVectors; ///< Stores all unique index vectors from the OPAT file, serving as the vertices of the triangulation.
        std::vector<std::vector<double>> m_axisValues; ///< Stores the unique values for each axis/dimension (Not currently used by Delaunay approach).
        std::size_t m_numCorners{}; ///< The number of corners in a hypercube (2^m_indexVectorSize), relevant for hypercube-based approaches (not Delaunay).
        std::vector<std::vector<std::size_t>> m_simplices; ///< Stores the simplices of the Delaunay triangulation. Each inner vector is a list of global vertex indices (indices into `m_indexVectors`).
        std::vector<std::vector<std::size_t>> m_simplexAdjacency; ///< Adjacency list for simplices. `m_simplexAdjacency[i][j]` stores the ID of the simplex adjacent to simplex `i` across the face opposite to its `j`-th local vertex. A value of `static_cast<std::size_t>(-1)` indicates no neighbor (boundary).

        /**
         * @brief Initializes the TableLattice internal structures.
         *
         * Extracts unique index vectors from the OPAT object and stores them in `m_indexVectors`.
         * Sets `m_indexVectorSize` based on OPAT header.
         * This method is called by the constructors.
         */
        void initialize();

        /**
         * @brief Builds the Delaunay triangulation of the index vectors.
         *
         * Uses the Qhull library to perform Delaunay triangulation on the points stored in `m_indexVectors`.
         * Populates `m_simplices` with the vertex indices of each simplex and `m_simplexAdjacency`
         * with neighbor information.
         * This method is called by the constructors after `initialize()`.
         * @throws std::runtime_error (wrapping `orgQhull::QhullError`) if Qhull encounters an error during triangulation.
         *         This can happen due to issues like insufficient points for the dimension,
         *         all points being collinear/coplanar in a way that prevents triangulation, or other Qhull internal errors.
         *         Resolution: Check the validity and distribution of index vectors in the input OPAT file.
         *                     Ensure there are enough non-degenerate points for the given number of dimensions.
         */
        void buildDelaunay();
        /**
         * @brief Finds the simplex containing the given index vector using a walk algorithm.
         *
         * Starts from a known simplex (potentially the last found one, `m_lastFoundSimplex`) and "walks"
         * through adjacent simplices until the one containing the `indexVector` is found.
         * @param indexVector The index vector to locate.
         * @return A `Simplex` struct containing the ID of the simplex and the barycentric weights of the `indexVector` within it.
         * @throws std::out_of_range if `validateIndexVector` fails (point outside overall bounds), or if the walk
         *         terminates at the convex hull and the point is determined to be outside. Also thrown for internal
         *         out-of-bounds access to `m_simplices`, `m_indexVectors`, or `m_simplexAdjacency`.
         *         Resolution: Ensure `indexVector` is within the data bounds and convex hull. Internal errors suggest issues with triangulation data.
         * @throws std::invalid_argument if `validateIndexVector` fails (dimension mismatch) or if `indexVector`
         *         dimension doesn't match `m_indexVectorSize` during the walk.
         *         Resolution: Ensure `indexVector` has the correct number of dimensions.
         * @throws std::runtime_error if the triangulation is empty (`m_simplices.empty()`), if a cycle is detected
         *         during the walk, if the walk cannot determine an exit face from a simplex, if the maximum
         *         number of walk steps is exceeded, or if `calculateBarycentricWeights` fails (e.g. degenerate simplex).
         *         Resolution: These often indicate issues with the triangulation's integrity, adjacency information,
         *                     numerical precision challenges, or a query point far outside the domain leading to extended walks.
         *                     Verify the input OPAT data.
         * @throws std::logic_error from `calculateBarycentricWeights` if an unexpected number of weights is returned.
         */
        [[nodiscard]] Simplex findContainingSimplex(const FloatIndexVector& indexVector) const;

        /**
         * @brief Validates if the given index vector is within the global bounds of the table and has the correct dimension.
         * @param indexVector The index vector to validate.
         * @throws std::invalid_argument if `indexVector.size()` does not match `m_opat.header.numIndex`.
         *         Resolution: Ensure the query `indexVector` has the same number of dimensions as the OPAT data.
         * @throws std::out_of_range if any component of `indexVector` is outside the min/max bounds
         *         defined by `m_opat.getBounds()` for that dimension.
         *         Resolution: Ensure all components of the `indexVector` are within the established bounds of the OPAT data.
         *
         * **Example:**
         * @code
         * // Assuming 'lattice' is an initialized TableLattice object
         * // and 'opat_data' is the OPAT object it was built from.
         * FloatIndexVector valid_point({0.5, 0.5}); // Assuming 2D and bounds contain this
         *
         * lattice.validateIndexVector(valid_point); // Should pass
         * // lattice.validateIndexVector(FloatIndexVector({0.5})); // Would throw std::invalid_argument if numIndex != 1
         * // lattice.validateIndexVector(FloatIndexVector({100.0, 100.0})); // Would throw std::out_of_range if bounds are smaller
         * @endcode
         */
        void validateIndexVector(const FloatIndexVector &indexVector) const;

        /**
         * @brief Calculates the barycentric weights of a query point with respect to the vertices of a given simplex.
         *
         * Solves a linear system of equations to find the weights.
         * The sum of barycentric weights is 1. If a point is inside a simplex, all its weights are non-negative.
         * @param queryPoint The point for which to calculate barycentric weights.
         * @param simplexActualVertices A vector of `FloatIndexVector` representing the coordinates of the simplex's vertices.
         *        The number of vertices must be `m_indexVectorSize + 1`.
         * @return A vector of `double` containing the barycentric weights. The order of weights corresponds to the
         *         order of vertices in `simplexActualVertices`.
         * @throws std::invalid_argument if `simplexActualVertices` does not contain `m_indexVectorSize + 1` vertices,
         *         or if `queryPoint` or any vertex in `simplexActualVertices` does not have `m_indexVectorSize` dimensions.
         *         Resolution: Ensure input vectors have correct sizes and dimensions matching the lattice.
         * @throws std::runtime_error if `solveLinearSystem` fails, typically because the matrix formed by simplex
         *         vertices is singular (e.g., degenerate simplex - vertices are collinear/coplanar).
         *         Resolution: This indicates an issue with the geometry of the provided simplex vertices.
         * @throws std::logic_error if `solveLinearSystem` returns an unexpected number of solution components.
         *         Resolution: This indicates an internal logic error in the linear solver or its usage.
         */
        std::vector<double> calculateBarycentricWeights(const FloatIndexVector& queryPoint, const std::vector<FloatIndexVector>& simplexActualVertices) const;

        mutable Simplex m_lastFoundSimplex; ///< Stores the last found simplex (ID and barycentric weights) as a starting point for the `findContainingSimplex` walk algorithm, optimizing searches for spatially coherent query points. Initialized with an invalid ID.

    };

    /**
     * @brief Solves a linear system of equations Ax = b.
     *
     * Uses LU decomposition to solve the system.
     * @param A The matrix A.
     * @param b The vector b.
     * @return The solution vector x.
     * @throws std::runtime_error if matrix dimensions are mismatched or LU factorization fails.
     *
     * **Example:**
     * @code
     * // Solves Ax = b for x
     * opat::lattice::bmat A(2, 2); // 2x2 matrix
     * A(0, 0) = 2.0; A(0, 1) = 1.0;
     * A(1, 0) = 1.0; A(1, 1) = 3.0;
     *
     * opat::lattice::bvec b(2);    // 2-element vector
     * b(0) = 5.0;
     * b(1) = 7.0;
     *
     * opat::lattice::bvec x = opat::lattice::solveLinearSystem(A, b);
     * // x should be approximately [1.6, 1.8]
     * std::cout << "Solution x: " << x << std::endl;
     * @endcode
     */
    bvec solveLinearSystem(bmat A, bvec b);
}
