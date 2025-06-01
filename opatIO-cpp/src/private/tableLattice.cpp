#include <cmath>
#include <memory>
#include <utility>

#include "libqhullcpp/Qhull.h"

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/lu.hpp>

#include "tableLattice.h"

#include <algorithm>
#include <iostream>
#include <ranges>
#include <stdexcept>

#include "libqhullcpp/QhullFacetList.h"
#include "libqhullcpp/QhullVertexSet.h"

#include <iomanip>

namespace opat::lattice {
    TableLattice::TableLattice(const opat::OPAT &opat) : m_opat(opat){
        initialize();
        buildDelaunay();
    }

    TableLattice::TableLattice(const opat::OPAT &opat, const InterpolationType &interpolationType) : m_opat(opat) {
        m_interpolationType = interpolationType;
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

            QhullFacetList facets = qh.facetList();
            for (const auto &facet : facets) {
                if (!facet.isUpperDelaunay()) {
                    std::vector<std::size_t> simplex;
                    simplex.reserve(dims + 1);
                    for (const auto &vertex : facet.vertices()) {
                        simplex.push_back(static_cast<std::size_t>(vertex.point().id()));
                    }
                    m_simplices.push_back(std::move(simplex));
                }
            }
        }
        catch (QhullError const &e) {
            std::cerr << "QhullError: " << e.what() << std::endl;
            throw;
        }
    }

    ContainingSimplex TableLattice::findContainingSimplex(const FloatIndexVector &indexVector) const {
        const std::size_t dims = m_indexVectorSize;

        bmat A(dims, dims);
        bvec b(dims);

        for (std::size_t s = 0; s < m_simplices.size(); ++s) {
            auto const &simplex = m_simplices[s];
            auto const &x0 = m_indexVectors[simplex[0]];

            for (std::size_t i = 0; i < dims; ++i) {
                b(i) = indexVector[i] - x0[i];
                for (std::size_t k = 1; k <= dims; ++k) {
                    auto const &xk = m_indexVectors[simplex[k]];
                    A(i, k - 1) = xk[i] - x0[i];
                }
            }

            bvec lambda;
            try {
                lambda = solveLinearSystem(A, b);
            } catch (...) { // TODO: Improve this handling of exceptions
                continue; // Singular so skip
            }

            std::vector<double> w(dims + 1);
            double sum = 0.0;
            for (std::size_t k = 1; k <= dims; ++k) {
                w[k] = lambda(k-1);
                sum += w[k];
            }
            w[0] = 1.0 - sum;

            bool inside = true;
            for (double wi : w) {
                if (wi <= -1e12 || wi > 1.0 + 1e-12) {
                    inside = false;
                    break;
                }
            }
            if (inside) {
                return ContainingSimplex {s, std::move(w)};
            }
        }
        throw std::out_of_range("TableLattice::findContainingSimplex");
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
            int total = resultTable.N_R * resultTable.N_C * resultTable.m_vsize;

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
