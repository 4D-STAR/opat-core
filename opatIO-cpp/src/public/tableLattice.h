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
     * @brief Represents a corner of a hypercube in the index space.
     */
    struct HypercubeCorner {
        FloatIndexVector ID; ///< The unique identifier (index vector) of the corner.
        double weight;       ///< The weight associated with this corner, typically for interpolation.
        std::size_t ordering; ///< An ordering value for the corner.
        /**
         * @example
         * @code
         * opat::lattice::HypercubeCorner corner;
         * corner.ID = FloatIndexVector({1.0, 2.0});
         * corner.weight = 0.75;
         * corner.ordering = 0;
         * @endcode
         */
    };

    /**
     * @brief Represents a simplex containing a point, along with barycentric weights.
     * @example
     * @code
     * opat::lattice::ContainingSimplex simplex_info;
     * simplex_info.ID = 5; // Simplex ID
     * simplex_info.barycentricWeights = {0.25, 0.5, 0.25}; // Weights for a 2D simplex (triangle)
     * @endcode
     */
    struct ContainingSimplex {
        std::size_t ID;                     ///< The ID of the simplex in the triangulation.
        std::vector<double> barycentricWeights; ///< Barycentric weights of the point within this simplex.
    };

    /**
     * @brief Defines the type of interpolation to be used.
     */
    enum class InterpolationType {
        Linear,    ///< Linear interpolation.
        Quadratic, ///< Quadratic interpolation (Not yet implemented).
        Cubic      ///< Cubic interpolation (Not yet implemented).
        /**
         * @example
         * @code
         * opat::lattice::InterpolationType type = opat::lattice::InterpolationType::Linear;
         * if (type == opat::lattice::InterpolationType::Linear) {
         *     // Do something for linear interpolation
         * }
         * @endcode
         */
    };

    /**
     * @brief Represents a lattice structure for interpolating data from an OPAT object.
     *
     * The TableLattice class builds a Delaunay triangulation of the OPAT index points
     * and provides methods to interpolate data for a given index vector.
     * @example
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
     *     // Optionally, set a different interpolation type (if implemented)
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
     *     std::cout << "Interpolated DataCard retrieved." << std::endl;
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
         * and builds a Delaunay triangulation. Defaults to Linear interpolation.
         * @param opat The OPAT object containing the data.
         */
        explicit TableLattice(const opat::OPAT& opat);
        /**
         * @brief Constructs a TableLattice object from an OPAT object with a specified interpolation type.
         *
         * Initializes the lattice using the index vectors from the OPAT object
         * and builds a Delaunay triangulation.
         * @param opat The OPAT object containing the data.
         * @param interpolationType The type of interpolation to use.
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
         * @throws std::out_of_range if the indexVector is outside the bounds of the table.
         * @throws std::invalid_argument if the indexVector has an incorrect dimension.
         * @example
         * @code
         * // Assuming 'lattice' is an initialized TableLattice object
         * // and 'my_opat_object' is the OPAT object it was built from.
         * // For an OPAT with 2 index dimensions:
         * FloatIndexVector target_iv({0.25, 0.75});
         * try {
         *     opat::DataCard result = lattice.get(target_iv);
         *     // Use 'result' which contains interpolated data
         * } catch (const std::out_of_range& oor) {
         *     std::cerr << "Index vector is out of bounds: " << oor.what() << std::endl;
         * } catch (const std::invalid_argument& ia) {
         *     std::cerr << "Invalid index vector dimension: " << ia.what() << std::endl;
         * }
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
         */
        void setInterpolationType(InterpolationType interpolationType);

        /**
         * @brief Dumps the Delaunay triangulation to ASCII files.
         *
         * Creates two files: one for the points (vertices) and one for the simplices.
         * This is useful for visualizing or debugging the triangulation.
         * @param points_file The path to the file where points will be saved.
         * @param simplices_file The path to the file where simplices will be saved.
         * @example
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
        std::vector<FloatIndexVector> m_indexVectors; ///< Stores all unique index vectors from the OPAT file.
        std::vector<std::vector<double>> m_axisValues; ///< Stores the unique values for each axis/dimension (Not currently used).
        std::size_t m_numCorners{}; ///< The number of corners in a hypercube (2^m_indexVectorSize).
        std::vector<std::vector<std::size_t>> m_simplices; ///< Stores the simplices of the Delaunay triangulation, each simplex is a list of point indices.

        /**
         * @brief Initializes the TableLattice internal structures.
         *
         * Extracts index vectors from the OPAT object and sets up basic parameters.
         */
        void initialize();

        /**
         * @brief Builds the Delaunay triangulation of the index vectors.
         *
         * Uses Qhull library to perform Delaunay triangulation.
         */
        void buildDelaunay();
        /**
         * @brief Finds the simplex containing the given index vector.
         *
         * Iterates through all simplices and checks if the point is inside
         * by solving a linear system for barycentric coordinates.
         * @param indexVector The index vector to locate.
         * @return A ContainingSimplex struct with the ID of the simplex and barycentric weights.
         * @throws std::out_of_range if the indexVector is not found within any simplex (i.e., outside the convex hull).
         */
        [[nodiscard]] ContainingSimplex findContainingSimplex(const FloatIndexVector& indexVector) const;

        /**
         * @brief Validates if the given index vector is within the bounds of the table and has correct dimensions.
         * @param indexVector The index vector to validate.
         * @throws std::out_of_range if the indexVector is outside the defined bounds.
         * @throws std::invalid_argument if the indexVector has an incorrect dimension.
         */
        void validateIndexVector(const FloatIndexVector &indexVector) const;

    };

    /**
     * @brief Solves a linear system of equations Ax = b.
     *
     * Uses LU decomposition to solve the system.
     * @param A The matrix A.
     * @param b The vector b.
     * @return The solution vector x.
     * @throws std::runtime_error if matrix dimensions are mismatched or LU factorization fails.
     * @example
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
     * try {
     *     opat::lattice::bvec x = opat::lattice::solveLinearSystem(A, b);
     *     // x should be approximately [1.6, 1.8]
     *     std::cout << "Solution x: " << x << std::endl;
     * } catch (const std::runtime_error& e) {
     *     std::cerr << "Error solving linear system: " << e.what() << std::endl;
     * }
     * @endcode
     */
    bvec solveLinearSystem(bmat A, bvec b);
}
