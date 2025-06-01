#pragma once

#include <memory>
#include <vector>
#include <utility>

#include "opatIO.h"
#include "indexVector.h"

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>


namespace opat::lattice {

    typedef boost::numeric::ublas::matrix<double> bmat;
    typedef boost::numeric::ublas::vector<double> bvec;

    struct HypercubeCorner {
        FloatIndexVector ID;
        double weight;
        std::size_t ordering;
    };

    struct ContainingSimplex {
        std::size_t ID;
        std::vector<double> barycentricWeights;
    };

    enum class InterpolationType {Linear, Quadratic, Cubic};

    class TableLattice {
    public:
        explicit TableLattice(const opat::OPAT& opat);
        TableLattice(const opat::OPAT& opat, const InterpolationType& interpolationType);

        TableLattice(const TableLattice&) = default;
        TableLattice& operator=(const TableLattice&) = delete;
        TableLattice(TableLattice&&) noexcept = default;
        TableLattice& operator=(TableLattice&&) noexcept = delete;
        ~TableLattice() = default;

        DataCard get(const FloatIndexVector& indexVector) const;
        [[nodiscard]] InterpolationType getInterpolationType() const;
        void setInterpolationType(InterpolationType interpolationType);

        void dumpDriangulationToAscii(const std::string &points_file, const std::string &simplices_file) const;
    private:
        const opat::OPAT &m_opat;
        std::size_t m_indexVectorSize{};
        InterpolationType m_interpolationType{InterpolationType::Linear};
        std::vector<FloatIndexVector> m_indexVectors;
        std::vector<std::vector<double>> m_axisValues;
        std::size_t m_numCorners{};
        std::vector<std::vector<std::size_t>> m_simplices;

        void initialize();

        void buildDelaunay();
        ContainingSimplex findContainingSimplex(const FloatIndexVector& indexVector) const;

        void validateIndexVector(const FloatIndexVector &indexVector) const;

    };

    bvec solveLinearSystem(bmat A, bvec b);
}
