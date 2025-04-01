#include <vector>
#include <cstdint>
#include <cmath> // For std::pow and std::trunc
#include <stdexcept> // For exception handling

#include "indexVector.h"
#include "xxhash64.h"

// Constructor: Initializes the object with default hash precision of 8.
// The object is marked as uninitialized until a vector is provided.
FloatIndexVector::FloatIndexVector() : m_hashPrescision(8) {
    m_initialized = false;
}

// Constructor: Initializes the object with a given vector and default hash precision of 8.
// Throws an exception if the input vector is empty.
FloatIndexVector::FloatIndexVector(const std::vector<double>& vec) : m_hashPrescision(8) {
    if (vec.empty()) {
        throw std::invalid_argument("Input vector cannot be empty.");
    }

    // Scale factor is used to convert floating-point values to integers for hashing.
    double scaleFactor = std::pow(10.0, m_hashPrescision);
    m_vector.reserve(vec.size()); // Reserve memory to avoid reallocations.
    m_vectorInt.reserve(vec.size());

    // Populate the internal vector and its integer representation.
    for(const auto& val : vec) {
        m_vector.push_back(val);
        m_vectorInt.push_back(static_cast<uint64_t>(std::trunc(val * scaleFactor)));
    }

    m_initialized = true; // Mark the object as initialized.
}

// Constructor: Initializes the object with a given vector and custom hash precision.
// Throws exceptions for invalid hash precision or empty input vector.
FloatIndexVector::FloatIndexVector(const std::vector<double>& vec, int hashPrescision) : m_hashPrescision(hashPrescision) {
    if (hashPrescision <= 0) {
        throw std::invalid_argument("hashPrescision must be a positive integer.");
    }
    if (hashPrescision >= 14) {
        throw std::invalid_argument("hashPrescision must be less than 14.");
    }
    if (vec.empty()) {
        throw std::invalid_argument("Input vector cannot be empty.");
    }

    double scaleFactor = std::pow(10.0, m_hashPrescision);
    m_vector.reserve(vec.size());
    m_vectorInt.reserve(vec.size());

    for(const auto& val : vec) {
        m_vector.push_back(val);
        m_vectorInt.push_back(static_cast<uint64_t>(std::trunc(val * scaleFactor)));
    }

    m_initialized = true;
}

// Copy constructor: Creates a deep copy of another FloatIndexVector.
// Throws an exception if the input vector is empty.
FloatIndexVector::FloatIndexVector(const FloatIndexVector& other) : m_hashPrescision(other.m_hashPrescision) {
    if (other.m_vector.empty()) {
        throw std::invalid_argument("Input vector cannot be empty.");
    }

    m_vector.reserve(other.m_vector.size());
    m_vectorInt.reserve(other.m_vector.size());

    for(const auto& val : other.m_vector) {
        m_vector.push_back(val);
    }
    for (const auto& val : other.m_vectorInt) {
        m_vectorInt.push_back(val);
    }

    m_initialized = other.m_initialized;
}

// Assignment operator: Copies the contents of another FloatIndexVector.
// Handles self-assignment gracefully.
FloatIndexVector& FloatIndexVector::operator=(const FloatIndexVector& other) {
    if (this != &other) {
        m_vector = other.m_vector;
        m_vectorInt = other.m_vectorInt;
        m_hashPrescision = other.m_hashPrescision;
        m_initialized = other.m_initialized;
    }
    return *this;
}

// Equality operator: Compares two FloatIndexVector objects for equality.
// Ensures both objects are initialized and compares their integer representations.
bool FloatIndexVector::operator==(const FloatIndexVector& other) const {
    if (!m_initialized || !other.m_initialized) {
        return false; // Uninitialized objects cannot be compared.
    }
    if (m_vector.size() != other.m_vector.size()) {
        return false; // Vectors of different sizes cannot be equal.
    }
    for (size_t i = 0; i < m_vectorInt.size(); ++i) {
        if (m_vectorInt[i] != other.m_vectorInt[i]) { // Compare integer values
            return false;
        }
    }

    if (m_hashPrescision != other.m_hashPrescision) {
        return false; // Hash precision must also match.
    }
    return true;
}

// Inequality operator: Returns the negation of the equality operator.
bool FloatIndexVector::operator!=(const FloatIndexVector& other) const {
    return !(*this == other);
}

// Sets the hash precision before initialization.
// Throws an exception if the object is already initialized or if the precision is invalid.
void FloatIndexVector::setHashPrecision(int hashPrescision) {
    if (m_initialized) {
        throw std::runtime_error("Cannot set hash precision after initialization.");
    }
    if (hashPrescision <= 0) {
        throw std::invalid_argument("hashPrescision must be a positive integer.");
    }
    if (hashPrescision >= 14) {
        throw std::invalid_argument("hashPrescision must be less than 14.");
    }
    m_hashPrescision = hashPrescision;
}

// Retrieves the current hash precision.
// Throws an exception if the object is not initialized.
int FloatIndexVector::getHashPrecision() const {
    if (!m_initialized) {
        throw std::runtime_error("FloatIndexVector is not initialized.");
    }
    return m_hashPrescision;
}

// Initializes the object with a vector and custom hash precision.
// Throws an exception if the object is already initialized.
void FloatIndexVector::initialize(const std::vector<double>& vec, int hashPrescision) {
    if (m_initialized) {
        throw std::runtime_error("FloatIndexVector is already initialized.");
    }
    setHashPrecision(hashPrescision);
    setVector(vec);

    m_initialized = true;
}

// Initializes the object with a vector and default hash precision.
// Throws an exception if the object is already initialized.
void FloatIndexVector::initialize(const std::vector<double>& vec) {
    if (m_initialized) {
        throw std::runtime_error("FloatIndexVector is already initialized.");
    }
    setVector(vec);
    m_initialized = true;
}

// Retrieves the stored vector.
// Throws an exception if the object is not initialized.
std::vector<double> FloatIndexVector::getVector() const {
    if (!m_initialized) {
        throw std::runtime_error("FloatIndexVector is not initialized.");
    }
    return m_vector;
}

// Sets the internal vector before initialization.
// Throws an exception if the object is already initialized or if the input vector is empty.
void FloatIndexVector::setVector(const std::vector<double>& vec) {
    if (m_initialized) {
        throw std::runtime_error("Cannot set vector after initialization.");
    }
    if (vec.empty()) {
        throw std::invalid_argument("Input vector cannot be empty.");
    }
    m_vector = vec;
}

// Computes a hash value for the internal integer representation of the vector.
// Uses the XXHash64 algorithm for fast and reliable hashing.
// Throws an exception if the object is not initialized.
size_t FloatIndexVector::hash() const {
    if (!m_initialized) {
        throw std::runtime_error("FloatIndexVector is not initialized.");
    }
    const void* data = static_cast<const void*>(m_vectorInt.data());
    size_t sizeInBytes = m_vectorInt.size() * sizeof(uint64_t);
    uint64_t hash = XXHash64::hash(data, sizeInBytes, 0);
    return static_cast<size_t>(hash);
}

