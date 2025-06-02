#include <cmath>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>

#include "libqhullcpp/Qhull.h"

#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

#include "tableLattice.h"

#include <algorithm>
#include <iostream>
#include <ranges>
#include <stdexcept>

#include "libqhullcpp/QhullFacetList.h"
#include "libqhullcpp/QhullVertexSet.h"

#include <iomanip>

#include "libqhullcpp/QhullFacetSet.h"

namespace opat::lattice {
    TableLattice::TableLattice(const opat::OPAT &opat) : m_opat(opat){
        initialize();
        buildDelaunay();
    }

    TableLattice::TableLattice(const opat::OPAT &opat, const InterpolationType &interpolationType) : m_opat(opat) {
        m_interpolationType = interpolationType;
        if (m_interpolationType != InterpolationType::Linear) {
            throw std::runtime_error("Only Linear interpolation is currently implemented.");
        }
        initialize();
        buildDelaunay();
    }

    void TableLattice::initialize() {
        m_indexVectorSize = m_opat.header.numIndex;
        m_numCorners = static_cast<int>(std::pow(2, m_indexVectorSize));

        m_indexVectors.clear();
        m_indexVectors.reserve(m_opat.header.numTables);

        for (const auto &iv: m_opat.cardCatalog.tableIndex | std::views::keys) {
            m_indexVectors.push_back(iv);
        }

    }

    void TableLattice::buildDelaunay() {
        using orgQhull::Qhull;
        using orgQhull::QhullError;
        using orgQhull::QhullFacetList;
        using orgQhull::QhullVertexSet;
        using orgQhull::QhullFacetSet;
        using orgQhull::QhullFacet;
        using orgQhull::QhullVertex;

        const int dims = static_cast<int>(m_indexVectorSize);
        const int numPoints = static_cast<int>(m_indexVectors.size());

        std::vector<double> coords;
        coords.reserve(numPoints * dims);

        // pack all points into a flat array
        for (auto const &iv : m_indexVectors) {
            for (int i = 0; i < dims; ++i) {
                coords.push_back(iv[i]);
            }
        }

        try {
            Qhull qh;

            qh.runQhull("", dims, numPoints, coords.data(), "d Qt");
            m_simplices.clear();
            m_simplexAdjacency.clear();
            // fill m_simplexAdjacency with vectors of

            QhullFacetList facets = qh.facetList();
            std::vector<QhullFacet> qhullSimplices;
            std::map<void*, std::size_t> qhullFacetToIDMap;

            std::size_t currentSimplexID = 0 ;
            for (const auto &facet : facets) {
                if (!facet.isUpperDelaunay()) {
                    qhullSimplices.push_back(facet);
                    qhullFacetToIDMap[facet.getFacetT()] = currentSimplexID;

                    std::vector<std::size_t> simplexVertexIndices;
                    simplexVertexIndices.reserve(dims + 1);
                    for (const auto& vertex : facet.vertices()) {
                        simplexVertexIndices.push_back(static_cast<std::size_t>(vertex.point().id()));
                    }
                    m_simplices.push_back(std::move(simplexVertexIndices));
                    currentSimplexID++;
                }
            }
            m_simplexAdjacency.resize(m_simplices.size(), std::vector<std::size_t>(dims + 1, static_cast<std::size_t>(-1) /*NO_NEIGHBOR*/));

            for (std::size_t i = 0; i < qhullSimplices.size(); ++i) {
                const auto& qhSimplex = qhullSimplices[i];

                QhullVertexSet simplexVertices = qhSimplex.vertices();
                QhullFacetSet neighbors = qhSimplex.neighborFacets();

                for (const auto& neighbor : neighbors) {
                    if (neighbor.isGood()) {
                        auto map_it = qhullFacetToIDMap.find(neighbor.getFacetT());
                        if (map_it != qhullFacetToIDMap.end()) {
                            const std::size_t neighborID = map_it->second;
                            // find the shared face
                            QhullVertexSet neighborVertices = neighbor.vertices();

                            for (int k = 0; k < simplexVertices.count(); ++k) {
                                const auto& vertexOfCurrentSimplex = simplexVertices[k];
                                bool isSharedVertex = false;
                                for (int l = 0; l < neighborVertices.count(); ++l) {
                                    if (neighborVertices[l].point().id() == vertexOfCurrentSimplex.point().id()) {
                                        isSharedVertex = true;
                                        break;
                                    }
                                }
                                if (!isSharedVertex) {
                                    const std::size_t nonSharedVertexId = static_cast<std::size_t>(vertexOfCurrentSimplex.point().id());
                                    for (std::size_t localVertexIDX = 0; localVertexIDX <= dims; ++localVertexIDX) {
                                        if (m_simplices[i][localVertexIDX] == nonSharedVertexId) {
                                            m_simplexAdjacency[i][localVertexIDX] = neighborID;
                                            break;
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        catch (QhullError const &e) {
            std::cerr << "QhullError: " << e.what() << std::endl;
            throw;
        }
    }

    Simplex TableLattice::findContainingSimplex(const FloatIndexVector &queryPoint) const {
        validateIndexVector(queryPoint);

        if (m_simplices.empty()) {
            throw std::runtime_error("TableLattice::findContainingSimplex. No simplices found. Did you call buildDelaunay()?");
        }

        if (queryPoint.size() != m_indexVectorSize) {
            throw std::invalid_argument(
                "TableLattice::findContainingSimplex: Query point dimension (" + std::to_string(queryPoint.size()) +
                ") does not match lattice dimension (" + std::to_string(m_indexVectorSize) + ").");
        }

        constexpr double BARYCENTRIC_TOLERANCE = 1e-8; // Tolerance for barycentric coordinates
        const std::size_t MAX_WALK_STEPS = (m_simplices.size() > 0) ? (m_simplices.size() * 2) + 10 : 100;
        constexpr std::size_t NO_NEIGHBOR_SENTINEL = static_cast<std::size_t>(-1);

        // --- Step 1: Select Starting Simplex ---
        std::size_t currentSimplexID = 0; // Default to the first simplex.

        if (m_lastFoundSimplex.ID != NO_NEIGHBOR_SENTINEL && m_lastFoundSimplex.ID < m_simplices.size()) {
            currentSimplexID = m_lastFoundSimplex.ID;
            // PERF: Could add a check here: if queryPoint is "close" to the m_lastFoundSimplex's center
        } else if (m_simplices.empty()) {
            throw std::runtime_error("TableLattice::findContainingSimplex: Triangulation became empty unexpectedly.");
        }
        if (currentSimplexID >= m_simplices.size() && !m_simplices.empty()) {
            currentSimplexID = 0; // Fallback to 0 if the stored ID was somehow invalid.
        }

        // --- Step 2: Walking through the simplices ---
        std::set<std::size_t> visitedSimplicesInThisWalk; // For cycle detection

        for (std::size_t stepCount = 0; stepCount < MAX_WALK_STEPS; ++stepCount) {
            if (currentSimplexID >= m_simplices.size()) {
                throw std::out_of_range(
                    "TableLattice::findContainingSimplex: currentSimplexID (" + std::to_string(currentSimplexID) +
                    ") is out of bounds for m_simplices (size: " + std::to_string(m_simplices.size()) + "). Walk step: " + std::to_string(stepCount));

            }

            // Cycle detection
            if (visitedSimplicesInThisWalk.contains(currentSimplexID)) {
                throw std::runtime_error(
                    "TableLattice::findContainingSimplex: Cycle detected. Re-visited simplex ID " +
                    std::to_string(currentSimplexID) + " during walk. This may indicate issues with adjacency or numerical precision.");
            }
            visitedSimplicesInThisWalk.insert(currentSimplexID);


            // Get Current simplex's actual vertex coordinates
            const std::vector<std::size_t> currentSimplexGlobalVertexIDs = m_simplices[currentSimplexID];
            if (currentSimplexGlobalVertexIDs.size() != m_indexVectorSize + 1) {
                throw std::runtime_error(
                   "TableLattice::findContainingSimplex: Simplex " + std::to_string(currentSimplexID) +
                   " has incorrect number of vertices. Expected " + std::to_string(m_indexVectorSize + 1) +
                   ", got " + std::to_string(currentSimplexGlobalVertexIDs.size()));
            }

            std::vector<FloatIndexVector> currentSimplexActualVertices;
            currentSimplexActualVertices.reserve(m_indexVectorSize + 1);
            for (std::size_t globalVertexID : currentSimplexGlobalVertexIDs) {
                if (globalVertexID >= m_indexVectors.size()) {
                    throw std::out_of_range(
                        "TableLattice::findContainingSimplex: Global vertex ID " + std::to_string(globalVertexID) +
                        " (from simplex " + std::to_string(currentSimplexID) +
                        ") is out of bounds for m_indexVectors (size: " + std::to_string(m_indexVectors.size()) + ").");
                }
                currentSimplexActualVertices.push_back(m_indexVectors[globalVertexID]);
            }

            // Calculate barycentric coordinates of query point wrt current simplex
            std::vector<double> barycentricWeights = calculateBarycentricWeights(queryPoint, currentSimplexActualVertices);
            if (barycentricWeights.size() != m_indexVectorSize + 1) {
                throw std::runtime_error(
                    "TableLattice::findContainingSimplex: calculateBarycentricCoordinates returned incorrect number of weights for simplex " +
                    std::to_string(currentSimplexID) + ". Expected " + std::to_string(m_indexVectorSize + 1) +
                    ", got " + std::to_string(barycentricWeights.size()));
            }

            // Check termination condition (is query point inside the current simplex?)
            bool isInsideOrOnBoundary = true;
            for (const double w: barycentricWeights) {
                if (w < -BARYCENTRIC_TOLERANCE || w > 1.0 + BARYCENTRIC_TOLERANCE) {
                    isInsideOrOnBoundary = false;
                    break;
                }
            }

            if (isInsideOrOnBoundary) {
                // Found the containing simplex
                // Update m_lastFoundSimplex for optimization of future walks

                m_lastFoundSimplex.ID = currentSimplexID;
                m_lastFoundSimplex.barycentricWeights = barycentricWeights;

                // TODO : add some cleanup to the weights to make sure they are still clamped to [0, 1] and sum to 1.0

                return m_lastFoundSimplex;
            }

            // Determine the exit face (which way should the walk move)
            double mostNegativeWeight = 0.0;
            std::size_t localIndexOfVertexOppositeToExitFace = static_cast<std::size_t>(-1);

            for (std::size_t i =0; i < barycentricWeights.size(); i++) {
                if (barycentricWeights[i] < mostNegativeWeight) {
                    mostNegativeWeight = barycentricWeights[i];
                    localIndexOfVertexOppositeToExitFace = i;
                }
            }

            // Some edge case handling. If no weight was significantly negative but sum > 1 + TOL then the point is not
            // outside but not clearly behind a specific vertex.

            if (localIndexOfVertexOppositeToExitFace == static_cast<std::size_t>(-1) || mostNegativeWeight >= -BARYCENTRIC_TOLERANCE) {
                // TODO: Add a more robust approach here
                std::string weights_str;
                for (std::size_t i = 0; i < barycentricWeights.size(); ++i) {
                    weights_str += std::to_string(barycentricWeights[i]) + (i == barycentricWeights.size() - 1 ? "" : ", ");
                }
                throw std::runtime_error(
                    "TableLattice::findContainingSimplex: Point is outside simplex " + std::to_string(currentSimplexID) +
                    " but could not determine a clear exit face based on negative barycentric coordinates. Weights: [" + weights_str + "]");

            }

            // Get neighboring simplex ID
            if (currentSimplexID >= m_simplexAdjacency.size() || localIndexOfVertexOppositeToExitFace >= m_simplexAdjacency[currentSimplexID].size()) {
                throw std::out_of_range(
                    "TableLattice::findContainingSimplex: Adjacency lookup out of bounds. SimplexID: " +
                    std::to_string(currentSimplexID) + ", FaceIndex: " + std::to_string(localIndexOfVertexOppositeToExitFace) +
                    ". Adjacency table size for this simplex: " + (currentSimplexID < m_simplexAdjacency.size() ? std::to_string(m_simplexAdjacency[currentSimplexID].size()) : "N/A"));

            }

            std::size_t neighborSimplexID = m_simplexAdjacency[currentSimplexID][localIndexOfVertexOppositeToExitFace];

            // Handle convex hull boundary case

            if (neighborSimplexID == NO_NEIGHBOR_SENTINEL) {
                bool pointActuallyInThisHullSimplex = true;
                for (double w : barycentricWeights) {
                    if (w < -BARYCENTRIC_TOLERANCE || w > 1.0 + BARYCENTRIC_TOLERANCE) {
                        pointActuallyInThisHullSimplex = false;
                        break;
                    }
                }

                if (pointActuallyInThisHullSimplex) {
                    m_lastFoundSimplex.ID = currentSimplexID;
                    m_lastFoundSimplex.barycentricWeights = barycentricWeights;
                    return m_lastFoundSimplex;
                } else {
                    throw std::out_of_range(
                        "TableLattice::findContainingSimplex: Query point is outside the convex hull. "
                        "Walk terminated at hull simplex ID " + std::to_string(currentSimplexID) +
                        " attempting to cross face opposite local vertex " + std::to_string(localIndexOfVertexOppositeToExitFace));

                }
            }

            // Move to the neighbor simplex
            currentSimplexID = neighborSimplexID;
        }

        // --- Step 3: Loop Termination (Max Walk Steps Reached) ---
        // If the loops completes the walk without finding anything then terminate here
        throw std::runtime_error(
            "TableLattice::findContainingSimplex: Failed to find containing simplex for query point within maximum walk steps (" +
            std::to_string(MAX_WALK_STEPS) + "). Last simplex checked: ID " + std::to_string(currentSimplexID) +
            ". This may indicate a disconnected triangulation, issues with adjacency data, or extreme numerical precision challenges.");
    }

    void TableLattice::validateIndexVector(const FloatIndexVector &indexVector) const {
        std::vector<Bounds> bounds = m_opat.getBounds(); // PERF: This could be cached for a small performance improvement
        if (indexVector.size() != m_opat.header.numIndex) {
            std::stringstream ss;
            ss << "TableLattice::indexVectorIsInTableBounds. Invalid query index vector dimension. ";
            ss << "Query index vector size is " << indexVector.size();
            ss << ", Expected size is " << m_opat.header.numIndex;
            throw std::invalid_argument(ss.str());
        }
        for (int dim = 0; dim < m_opat.header.numIndex; ++dim) {
            if (indexVector[dim] < bounds.at(dim).min || indexVector[dim] > bounds.at(dim).max) {
                std::stringstream ss;
                ss << "TableLattice::indexVectorIsInTableBounds. Query index vector out of bounds. ";
                ss << "Query index vector is " << indexVector;
                ss << ", bounds are ";
                for (auto const& bound : bounds) {
                    ss << bound << ", ";
                }
                ss << "make sure that you are requesting values within the bounds!" << std::endl;
                throw std::out_of_range(ss.str());
            }
        }
    }

    std::vector<double> TableLattice::calculateBarycentricWeights(const FloatIndexVector &queryPoint,
                                                                  const std::vector<FloatIndexVector> &simplexActualVertices) const {
        const std::size_t N = m_indexVectorSize;

        // Input Validation
        if (simplexActualVertices.size() != N + 1) {
            throw std::invalid_argument(
                "calculateBarycentricWeights: Incorrect number of simplex vertices provided. Expected " +
                std::to_string(N + 1) + " (for N=" + std::to_string(N) + "), got " +
                std::to_string(simplexActualVertices.size()) + ".");

        }

        if (queryPoint.size() != N) {
            throw std::invalid_argument(
                "calculateBarycentricWeights: Query point dimension (" + std::to_string(queryPoint.size()) +
                ") does not match lattice dimension N (" + std::to_string(N) + ").");
        }
        // Validate dimensions of each vertex in the simplex
        for (std::size_t i = 0; i < simplexActualVertices.size(); ++i) {
            if (simplexActualVertices[i].size() != N) {
                throw std::invalid_argument(
                    "calculateBarycentricWeights: Simplex vertex " + std::to_string(i) + " has dimension " +
                    std::to_string(simplexActualVertices[i].size()) + ", but lattice dimension N is " + std::to_string(N) + ".");
            }
        }

        // TODO: could add a case to handle the 0 dimensional case (should not happen with qhull though)

        bmat M (N, N);
        bvec b(N);

        const FloatIndexVector& v0 = simplexActualVertices[0];

        for (std::size_t i = 0; i < N; ++i) {
            b(i) = queryPoint[i] - v0[i];
            for (std::size_t j = 0; j < N; ++j) {
                // Column j of M corresponds to the vector (V_{j+1} - v0)
                const FloatIndexVector& vj1 = simplexActualVertices[j + 1];
                M(i, j) = vj1[i] - v0[i];
            }
        }

        bvec solution;
        try {
            solution = solveLinearSystem(M, b);
        } catch (const std::runtime_error &e) {
            std::string error_message = "calculateBarycentricWeights: Failed to solve linear system for weights. "
                            "The simplex may be degenerate (e.g., vertices are collinear/coplanar), "
                            "leading to a singular matrix. Original error: ";
            error_message += e.what();
            throw std::runtime_error(error_message);
        }

        if (solution.size() != N) {
            throw std::logic_error(
                "calculateBarycentricWeights: Linear system solver returned an unexpected number of weights. Expected " +
                std::to_string(N) + ", got " + std::to_string(solution.size()) + ".");
        }

        // Calculate all N+1 barycentric weights
        std::vector<double> allBarycentricWeights(N + 1);
        double sumOfw1wN = 0.0;

        for (std::size_t j = 0; j < N; ++j) {
            double weightWjp1 = solution[j];
            allBarycentricWeights[j+1] = weightWjp1;
            sumOfw1wN += weightWjp1;
        }

        allBarycentricWeights[0] = 1.0 - sumOfw1wN;

        return allBarycentricWeights;
    }


    DataCard TableLattice::get(const FloatIndexVector &indexVector) const {
        validateIndexVector(indexVector);

        auto [ID, barycentricWeights] = findContainingSimplex(indexVector);
        auto const &simplex = m_simplices[ID];
        auto const &weights = barycentricWeights;

        FloatIndexVector iv0 = m_indexVectors[simplex[0]];
        const DataCard &baseDataCard = m_opat[iv0];

        DataCard resultDataCard;

        resultDataCard.header = baseDataCard.header;
        resultDataCard.tableIndex = baseDataCard.tableIndex;

        std::unordered_map<std::string, OPATTable> resultTables;
        for (const auto& key : baseDataCard.getKeys()) {
            const OPATTable &baseTable = baseDataCard[key];

            OPATTable resultTable;
            resultTable.N_R = baseTable.N_R;
            resultTable.N_C = baseTable.N_C;
            resultTable.m_vsize = baseTable.m_vsize;
            uint64_t total = resultTable.N_R * resultTable.N_C * resultTable.m_vsize;

            resultTable.rowValues = std::make_unique<double[]>(resultTable.N_R);
            resultTable.columnValues = std::make_unique<double[]>(resultTable.N_C);
            std::copy_n(baseTable.rowValues.get(), resultTable.N_R, resultTable.rowValues.get());
            std::copy_n(baseTable.columnValues.get(), resultTable.N_C, resultTable.columnValues.get());

            resultTable.data = std::make_unique<double[]>(total);
            std::fill_n(resultTable.data.get(), total, 0.0);

            for (std::size_t corner  = 0; corner < simplex.size(); ++corner) {
                const FloatIndexVector &iv = m_indexVectors[simplex[corner]];
                const OPATTable &cornerTable = m_opat[iv][key];
                const double *cornerData = cornerTable.data.get();
                double *resultData = resultTable.data.get();

                for (int idx = 0; idx < total; ++idx) {
                    resultData[idx] += weights[corner] * cornerData[idx];
                }
            }
            resultTables.emplace(key, std::move(resultTable));
        }
        resultDataCard.tableData = std::move(resultTables);
        return resultDataCard;
    }

    InterpolationType TableLattice::getInterpolationType() const {
        return m_interpolationType;
    }

    void TableLattice::setInterpolationType(InterpolationType interpolationType) {
        if (interpolationType != InterpolationType::Linear) {
            throw std::runtime_error("Only Linear interpolation is currently implemented.");
        }
        m_interpolationType = interpolationType;
    }

    void TableLattice::dumpTriangulationToAscii(const std::string &points_file, const std::string &simplices_file) const {
        {
            // 1) Emit the point cloud
            std::ofstream pts(points_file);
            pts << "# id";
            for (size_t i = 0; i < m_indexVectorSize; ++i)
                pts << " x" << i;
            pts << "\n";

            for (size_t id = 0; id < m_indexVectors.size(); ++id) {
                pts << id;
                for (std::size_t dim = 0; dim < m_indexVectorSize; ++dim) {
                    pts << " " << std::setprecision(8) << m_indexVectors[id][dim];
                }
                pts << "\n";
            }
            pts.close();

            // 2) Emit the simplices
            std::ofstream sfs(simplices_file);
            sfs << "# simplex vertices (by point id)\n";
            for (auto const &simplex : m_simplices) {
                for (size_t vid : simplex) {
                    sfs << vid << " ";
                }
                sfs << "\n";
            }
            sfs.close();
        }
    }

    bvec solveLinearSystem(bmat A, bvec b) {
        using boost::numeric::ublas::permutation_matrix;

        const std::size_t n = A.size1();
        if (b.size() != n || A.size2() != n) {
            throw std::runtime_error("Dimension mismatch in opat::lattice::solveLinearSystem");
        }

        permutation_matrix<std::size_t> pivots(n);
        if (lu_factorize(A, pivots) != 0) {
            throw std::runtime_error("Lu factorization failed (singular matrix A) in opat::lattice::solveLinearSystem");
        }

        lu_substitute(A, pivots, b);

        return b;
    }

}
