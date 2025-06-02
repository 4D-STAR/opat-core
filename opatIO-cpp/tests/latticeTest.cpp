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
    EXPECT_THROW(lattice.setInterpolationType(opat::lattice::InterpolationType::Quadratic), std::runtime_error);
    EXPECT_EQ(lattice.getInterpolationType(), opat::lattice::InterpolationType::Linear);

}

TEST_F(tableLatticeTest, get) {
    const opat::OPAT opatObj = opat::readOPAT(EXAMPLE_FILENAME);
    opat::lattice::TableLattice lattice(opatObj);
    FloatIndexVector targetVector({0.54421, 0.077585});
    EXPECT_NO_THROW(opat::DataCard interpolated = lattice.get(targetVector));
    FloatIndexVector targetVector2({0.54421, 0.77585});
    EXPECT_THROW(auto r = lattice.get(targetVector2), std::out_of_range);
}

TEST_F(tableLatticeTest, exactInterpolation) {
    const opat::OPAT opatObj = opat::readOPAT(EXAMPLE_FILENAME);
    opat::lattice::TableLattice lattice(opatObj);
    FloatIndexVector targetVector({0.2, 0.06});
    opat::DataCard interpolated = lattice.get(targetVector);
    const opat::DataCard &expected = opatObj[targetVector];

    for (int row = 0; row < expected["data"].size().first; ++row) {
        for (int col = 0; col < expected["data"].size().second; ++col) {
            // Check for nan in expected or interpolated
            if (std::isnan(expected["data"](row, col, 0)) || std::isnan(interpolated["data"](row, col, 0))) {
                // assert both are nan
                bool expectedIsNan = std::isnan(expected["data"](row, col, 0));
                bool interpolatedIsNan = std::isnan(interpolated["data"](row, col, 0));
                EXPECT_TRUE(expectedIsNan && interpolatedIsNan) << "Row: " << row << ", Col: " << col
                                                                << " Expected NaN: " << expectedIsNan
                                                                << ", Interpolated NaN: " << interpolatedIsNan;
            } else {
                EXPECT_NEAR(interpolated["data"](row, col, 0), expected["data"](row, col, 0), 1e-8);
            }
        }
    }
}

TEST_F(tableLatticeTest, interpolationOffControlPointDim1) {
    const opat::OPAT opatObj = opat::readOPAT(EXAMPLE_FILENAME);
    opat::lattice::TableLattice lattice(opatObj);
    FloatIndexVector targetVector({0.275, 0.06});
    opat::DataCard interpolated = lattice.get(targetVector);
    const opat::DataCard& leftExpected = opatObj[FloatIndexVector({0.2, 0.06})];
    const opat::DataCard& rightExpected = opatObj[FloatIndexVector({0.35, 0.06})];
    for (int row = 0; row < leftExpected["data"].size().first; ++row) {
        for (int col = 0; col < leftExpected["data"].size().second; ++col) {
            // Check for nan in expected or interpolated
            if (std::isnan(leftExpected["data"](row, col, 0)) || std::isnan(interpolated["data"](row, col, 0)) ||
                std::isnan(rightExpected["data"](row, col, 0))) {
                // assert both are nan
                bool leftIsNan = std::isnan(leftExpected["data"](row, col, 0));
                bool rightIsNan = std::isnan(rightExpected["data"](row, col, 0));
                bool interpolatedIsNan = std::isnan(interpolated["data"](row, col, 0));
                EXPECT_TRUE(leftIsNan && rightIsNan && interpolatedIsNan)
                    << "Row: " << row << ", Col: " << col
                    << " Left NaN: " << leftIsNan
                    << ", Right NaN: " << rightIsNan
                    << ", Interpolated NaN: " << interpolatedIsNan;
            } else {
                double expectedValue = (leftExpected["data"](row, col, 0) + rightExpected["data"](row, col, 0)) / 2.0;
                EXPECT_NEAR(interpolated["data"](row, col, 0), expectedValue, 1e-8);
            }
        }
    }
}
TEST_F(tableLatticeTest, interpolationOffControlPointDim2) {
    const opat::OPAT opatObj = opat::readOPAT(EXAMPLE_FILENAME);
    opat::lattice::TableLattice lattice(opatObj);
    FloatIndexVector targetVector({0.2, 0.07});
    opat::DataCard interpolated = lattice.get(targetVector);
    const opat::DataCard& lowerExpected = opatObj[FloatIndexVector({0.2, 0.06})];
    const opat::DataCard& upperExpected = opatObj[FloatIndexVector({0.2, 0.08})];
    for (int row = 0; row < lowerExpected["data"].size().first; ++row) {
        for (int col = 0; col < lowerExpected["data"].size().second; ++col) {
            // Check for nan in expected or interpolated
            if (std::isnan(lowerExpected["data"](row, col, 0)) || std::isnan(interpolated["data"](row, col, 0)) ||
                std::isnan(upperExpected["data"](row, col, 0))) {
                // assert both are nan
                bool leftIsNan = std::isnan(lowerExpected["data"](row, col, 0));
                bool rightIsNan = std::isnan(upperExpected["data"](row, col, 0));
                bool interpolatedIsNan = std::isnan(interpolated["data"](row, col, 0));
                EXPECT_TRUE(leftIsNan && rightIsNan && interpolatedIsNan)
                    << "Row: " << row << ", Col: " << col
                    << " Lower NaN: " << leftIsNan
                    << ", Upper NaN: " << rightIsNan
                    << ", Interpolated NaN: " << interpolatedIsNan;
                } else {
                    double expectedValue = (lowerExpected["data"](row, col, 0) + upperExpected["data"](row, col, 0)) / 2.0;
                    EXPECT_NEAR(interpolated["data"](row, col, 0), expectedValue, 1e-8);
                }
        }
    }
}

TEST_F(tableLatticeTest, bilinearInterpolation) {
    const opat::OPAT opatObj = opat::readOPAT(EXAMPLE_FILENAME);
    opat::lattice::TableLattice lattice(opatObj);
    FloatIndexVector targetVector({0.275, 0.07});
    opat::DataCard interpolated = lattice.get(targetVector);

// At least on my computer (arch arm64, Apple M4 pro) gcc and clang produce triangulations with slightly different
// connectivity. This does not affect the correctness of this code; however, when making simple comparisons to control
// points, we need to account for this difference. The control points are chosen such that they are guaranteed to be
// present in the triangulation. This is cludgy but works for now.
#if defined(__clang__)
    const opat::DataCard& lowerExpected = opatObj[FloatIndexVector({0.2, 0.06})];
    const opat::DataCard& upperExpected = opatObj[FloatIndexVector({0.35, 0.08})];
#elif defined(__GNUC__) || defined(__GNUG__) && !defined(__clang__)
    const opat::DataCard& lowerExpected = opatObj[FloatIndexVector({0.35, 0.06})];
    const opat::DataCard& upperExpected = opatObj[FloatIndexVector({0.2, 0.08})];
#else
    GTEST_SKIP();
#endif

    for (int row = 0; row < lowerExpected["data"].size().first; ++row) {
        for (int col = 0; col < lowerExpected["data"].size().second; ++col) {
            // Check for nan in expected or interpolated
            if (std::isnan(lowerExpected["data"](row, col, 0)) || std::isnan(interpolated["data"](row, col, 0)) ||
                std::isnan(upperExpected["data"](row, col, 0))) {
                // assert both are nan
                bool leftIsNan = std::isnan(lowerExpected["data"](row, col, 0));
                bool rightIsNan = std::isnan(upperExpected["data"](row, col, 0));
                bool interpolatedIsNan = std::isnan(interpolated["data"](row, col, 0));
                EXPECT_TRUE(leftIsNan && rightIsNan && interpolatedIsNan)
                    << "Row: " << row << ", Col: " << col
                    << " Lower Left NaN: " << leftIsNan
                    << ", Upper Right NaN: " << rightIsNan
                    << ", Interpolated NaN: " << interpolatedIsNan;
            } else {
                double expectedValue = (lowerExpected["data"](row, col, 0) + upperExpected["data"](row, col, 0)) / 2.0;
                EXPECT_NEAR(interpolated["data"](row, col, 0), expectedValue, 1e-8);
            }
        }
    }
}

TEST_F(tableLatticeTest, RightBoundaryBehaviorAtControlPoint) {
    opat::OPAT opatObj = opat::readOPAT(EXAMPLE_FILENAME);
    opat::lattice::TableLattice lattice(opatObj);
    FloatIndexVector targetVector({0.94, 0.06});
    opat::DataCard interpolated = lattice.get(targetVector);
    const opat::DataCard& expected = opatObj[targetVector];
    for (int row = 0; row < expected["data"].size().first; ++row) {
        for (int col = 0; col < expected["data"].size().second; ++col) {
            // Check for nan in expected or interpolated
            if (std::isnan(expected["data"](row, col, 0)) || std::isnan(interpolated["data"](row, col, 0))) {
                // assert both are nan
                bool expectedIsNan = std::isnan(expected["data"](row, col, 0));
                bool interpolatedIsNan = std::isnan(interpolated["data"](row, col, 0));
                EXPECT_TRUE(expectedIsNan && interpolatedIsNan) << "Row: " << row << ", Col: " << col
                                                                << " Expected NaN: " << expectedIsNan
                                                                << ", Interpolated NaN: " << interpolatedIsNan;
            } else {
                EXPECT_NEAR(interpolated["data"](row, col, 0), expected["data"](row, col, 0), 1e-8);
            }
        }
    }
}

TEST_F(tableLatticeTest, LeftBoundaryBehaviorAtControlPoint) {
    opat::OPAT opatObj = opat::readOPAT(EXAMPLE_FILENAME);
    opat::lattice::TableLattice lattice(opatObj);
    FloatIndexVector targetVector({0.00, 0.06});
    opat::DataCard interpolated = lattice.get(targetVector);
    const opat::DataCard& expected = opatObj[targetVector];
    for (int row = 0; row < expected["data"].size().first; ++row) {
        for (int col = 0; col < expected["data"].size().second; ++col) {
            // Check for nan in expected or interpolated
            if (std::isnan(expected["data"](row, col, 0)) || std::isnan(interpolated["data"](row, col, 0))) {
                // assert both are nan
                bool expectedIsNan = std::isnan(expected["data"](row, col, 0));
                bool interpolatedIsNan = std::isnan(interpolated["data"](row, col, 0));
                EXPECT_TRUE(expectedIsNan && interpolatedIsNan) << "Row: " << row << ", Col: " << col
                                                                << " Expected NaN: " << expectedIsNan
                                                                << ", Interpolated NaN: " << interpolatedIsNan;
            } else {
                EXPECT_NEAR(interpolated["data"](row, col, 0), expected["data"](row, col, 0), 1e-8);
            }
        }
    }
}

TEST_F(tableLatticeTest, UpperBoundaryBehaviorAtControlPoint) {
    opat::OPAT opatObj = opat::readOPAT(EXAMPLE_FILENAME);
    opat::lattice::TableLattice lattice(opatObj);
    FloatIndexVector targetVector({0.2, 0.10});
    opat::DataCard interpolated = lattice.get(targetVector);
    const opat::DataCard& expected = opatObj[targetVector];
    for (int row = 0; row < expected["data"].size().first; ++row) {
        for (int col = 0; col < expected["data"].size().second; ++col) {
            // Check for nan in expected or interpolated
            if (std::isnan(expected["data"](row, col, 0)) || std::isnan(interpolated["data"](row, col, 0))) {
                // assert both are nan
                bool expectedIsNan = std::isnan(expected["data"](row, col, 0));
                bool interpolatedIsNan = std::isnan(interpolated["data"](row, col, 0));
                EXPECT_TRUE(expectedIsNan && interpolatedIsNan) << "Row: " << row << ", Col: " << col
                                                                << " Expected NaN: " << expectedIsNan
                                                                << ", Interpolated NaN: " << interpolatedIsNan;
            } else {
                EXPECT_NEAR(interpolated["data"](row, col, 0), expected["data"](row, col, 0), 1e-8);
            }
        }
    }
}

TEST_F(tableLatticeTest, LowerBoundaryBehaviorAtControlPoint) {
    opat::OPAT opatObj = opat::readOPAT(EXAMPLE_FILENAME);
    opat::lattice::TableLattice lattice(opatObj);
    FloatIndexVector targetVector({0.2, 0.00});
    opat::DataCard interpolated = lattice.get(targetVector);
    const opat::DataCard& expected = opatObj[targetVector];
    for (int row = 0; row < expected["data"].size().first; ++row) {
        for (int col = 0; col < expected["data"].size().second; ++col) {
            // Check for nan in expected or interpolated
            if (std::isnan(expected["data"](row, col, 0)) || std::isnan(interpolated["data"](row, col, 0))) {
                // assert both are nan
                bool expectedIsNan = std::isnan(expected["data"](row, col, 0));
                bool interpolatedIsNan = std::isnan(interpolated["data"](row, col, 0));
                EXPECT_TRUE(expectedIsNan && interpolatedIsNan) << "Row: " << row << ", Col: " << col
                                                                << " Expected NaN: " << expectedIsNan
                                                                << ", Interpolated NaN: " << interpolatedIsNan;
            } else {
                EXPECT_NEAR(interpolated["data"](row, col, 0), expected["data"](row, col, 0), 1e-8);
            }
        }
    }
}

TEST_F(tableLatticeTest, LowerBoundaryBehaviorBetweenControlPoints) {
    opat::OPAT opatObj = opat::readOPAT(EXAMPLE_FILENAME);
    opat::lattice::TableLattice lattice(opatObj);
    FloatIndexVector targetVector({0.6, 0.00});
    opat::DataCard interpolated = lattice.get(targetVector);

    FloatIndexVector leftVector({0.5, 0.00});
    FloatIndexVector rightVector({0.7, 0.00});
    const opat::DataCard& leftExpected = opatObj[leftVector];
    const opat::DataCard& rightExpected = opatObj[rightVector];

    for (int row = 0; row < leftExpected["data"].size().first; ++row) {
        for (int col = 0; col < leftExpected["data"].size().second; ++col) {
            // Check for nan in expected or interpolated
            if (std::isnan(leftExpected["data"](row, col, 0)) || std::isnan(interpolated["data"](row, col, 0)) ||
                std::isnan(rightExpected["data"](row, col, 0))) {
                // assert both are nan
                bool leftIsNan = std::isnan(leftExpected["data"](row, col, 0));
                bool rightIsNan = std::isnan(rightExpected["data"](row, col, 0));
                bool interpolatedIsNan = std::isnan(interpolated["data"](row, col, 0));
                EXPECT_TRUE(leftIsNan && rightIsNan && interpolatedIsNan)
                    << "Row: " << row << ", Col: " << col
                    << " Left NaN: " << leftIsNan
                    << ", Right NaN: " << rightIsNan
                    << ", Interpolated NaN: " << interpolatedIsNan;
            } else {
                double expectedValue = (leftExpected["data"](row, col, 0) + rightExpected["data"](row, col, 0)) / 2.0;
                EXPECT_NEAR(interpolated["data"](row, col, 0), expectedValue, 1e-8);
            }
        }
    }
}

TEST_F(tableLatticeTest, degenerateControlPoint) {
    opat::OPAT opatObj = opat::readOPAT(EXAMPLE_FILENAME);
    opat::lattice::TableLattice lattice(opatObj);
    FloatIndexVector targetVector({0.95, 0.00});
    opat::DataCard interpolated = lattice.get(targetVector);
    const opat::DataCard& expected = opatObj[targetVector];
    for (int row = 0; row < expected["data"].size().first; ++row) {
        for (int col = 0; col < expected["data"].size().second; ++col) {
            // Check for nan in expected or interpolated
            if (std::isnan(expected["data"](row, col, 0)) || std::isnan(interpolated["data"](row, col, 0))) {
                // assert both are nan
                bool expectedIsNan = std::isnan(expected["data"](row, col, 0));
                bool interpolatedIsNan = std::isnan(interpolated["data"](row, col, 0));
                EXPECT_TRUE(expectedIsNan && interpolatedIsNan) << "Row: " << row << ", Col: " << col
                                                                << " Expected NaN: " << expectedIsNan
                                                                << ", Interpolated NaN: " << interpolatedIsNan;
            } else {
                EXPECT_NEAR(interpolated["data"](row, col, 0), expected["data"](row, col, 0), 1e-8);
            }
        }
    }
}

TEST_F(tableLatticeTest, nearDegenerateControlPoint) {
    opat::OPAT opatObj = opat::readOPAT(EXAMPLE_FILENAME);
    opat::lattice::TableLattice lattice(opatObj);
    FloatIndexVector targetVector({0.95, 0.00005});
    opat::DataCard interpolated = lattice.get(targetVector);

    FloatIndexVector upperVector({0.95, 0.0001});
    FloatIndexVector lowerVector({0.95, 0.0});

    const opat::DataCard& upperExpected = opatObj[upperVector];
    const opat::DataCard& lowerExpected = opatObj[lowerVector];

    for (int row = 0; row < upperExpected["data"].size().first; ++row) {
        for (int col = 0; col < upperExpected["data"].size().second; ++col) {
            // Check for nan in expected or interpolated
            if (std::isnan(upperExpected["data"](row, col, 0)) || std::isnan(interpolated["data"](row, col, 0)) ||
                std::isnan(lowerExpected["data"](row, col, 0))) {
                // assert both are nan
                bool upperIsNan = std::isnan(upperExpected["data"](row, col, 0));
                bool lowerIsNan = std::isnan(lowerExpected["data"](row, col, 0));
                bool interpolatedIsNan = std::isnan(interpolated["data"](row, col, 0));
                EXPECT_TRUE(upperIsNan && lowerIsNan && interpolatedIsNan)
                    << "Row: " << row << ", Col: " << col
                    << " Upper NaN: " << upperIsNan
                    << ", Lower NaN: " << lowerIsNan
                    << ", Interpolated NaN: " << interpolatedIsNan;
            } else {
                double expectedValue = (upperExpected["data"](row, col, 0) + lowerExpected["data"](row, col, 0)) / 2.0;
                EXPECT_NEAR(interpolated["data"](row, col, 0), expectedValue, 1e-8);
            }
        }
    }
}

TEST_F(tableLatticeTest, outputUtility_thisTestDoesNotTestAnything) {
    opat::OPAT opatObj = opat::readOPAT(EXAMPLE_FILENAME);
    opat::lattice::TableLattice lattice(opatObj);

    lattice.dumpTriangulationToAscii("points.txt", "simplices.txt");

}
