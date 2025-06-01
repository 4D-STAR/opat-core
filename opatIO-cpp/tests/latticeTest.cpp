#include <gtest/gtest.h>
#include "opatIO.h"
#include "tableLattice.h"
#include "indexVector.h"

#include <string>

std::string EXAMPLE_FILENAME = std::string(getenv("MESON_SOURCE_ROOT")) + "/opatIO-cpp/tests/gs98hz.opat";

class tableLatticeTest : public ::testing::Test {};

TEST_F(tableLatticeTest, DefaultConstructor) {
    opat::OPAT opatObj = opat::readOPAT(EXAMPLE_FILENAME);
    EXPECT_NO_THROW(opat::lattice::TableLattice lattice(opatObj));
}

TEST_F(tableLatticeTest, getInterpolationType) {
    const opat::OPAT opatObj = opat::readOPAT(EXAMPLE_FILENAME);
    const opat::lattice::TableLattice lattice(opatObj);
    EXPECT_EQ(lattice.getInterpolationType(), opat::lattice::InterpolationType::Linear);
}

TEST_F(tableLatticeTest, setInterpolationType) {
    const opat::OPAT opatObj = opat::readOPAT(EXAMPLE_FILENAME);
    opat::lattice::TableLattice lattice(opatObj);
    EXPECT_NO_THROW(lattice.setInterpolationType(opat::lattice::InterpolationType::Quadratic));
    EXPECT_EQ(lattice.getInterpolationType(), opat::lattice::InterpolationType::Quadratic);

}

TEST_F(tableLatticeTest, get) {
    const opat::OPAT opatObj = opat::readOPAT(EXAMPLE_FILENAME);
    opat::lattice::TableLattice lattice(opatObj);
    FloatIndexVector targetVector({0.54421, 0.077585});
    lattice.dumpDriangulationToAscii("points.dat", "simplices.dat");
    EXPECT_NO_THROW(opat::DataCard interpolated = lattice.get(targetVector));
    FloatIndexVector targetVector2({0.54421, 0.77585});
    EXPECT_THROW(lattice.get(targetVector2), std::out_of_range);
}
