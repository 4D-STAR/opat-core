#pragma once

#include <stdexcept>
#include <vector>
#include <functional>

/**
 * @file indexVector.h
 * @brief Header file defining the FloatIndexVector class for handling floating-point index vectors.
 *
 * This class is designed to provide a robust and efficient way to handle floating-point vectors
 * with support for hashing and precision control. It is primarily used in the OPAT library for
 * indexing and cataloging data tables.
 *
 * Design Choices:
 * - **Precision Control**: Allows rounding of floating-point values to a specified precision for consistent hashing.
 * - **Hashing Support**: Implements a custom hash function for use in unordered containers.
 * - **Flexibility**: Supports initialization with or without precision and provides methods for modifying the vector.
 *
 * Example Usage:
 * @code
 * #include "indexVector.h"
 * 
 * int main() {
 *     std::vector<double> vec = {1.2345, 2.3456, 3.4567};
 *     FloatIndexVector index(vec, 2); // Initialize with precision of 2 decimal places
 * 
 *     std::unordered_map<FloatIndexVector, std::string> map;
 *     map[index] = "Example Data";
 * 
 *     if (map.find(index) != map.end()) {
 *         std::cout << "Found: " << map[index] << std::endl;
 *     }
 *     return 0;
 * }
 * @endcode
 */
class FloatIndexVector {
public:
    /**
     * @brief Default constructor.
     *
     * Initializes an empty FloatIndexVector.
     */
    FloatIndexVector();

    /**
     * @brief Constructor with vector initialization.
     * @param vec The vector of floating-point values to initialize with.
     * @throws std::invalid_argument if the input vector is empty.
     */
    FloatIndexVector(const std::vector<double>& vec);

    /**
     * @brief Constructor with vector and precision initialization.
     * @param vec The vector of floating-point values to initialize with.
     * @param hashPrecision The precision to use for hashing.
     * @throws std::invalid_argument if the input vector is empty.
     * @throws std::invalid_argument if hashPrescision is not a positive integer or is >= 14.
     */
    FloatIndexVector(const std::vector<double>& vec, int hashPrecision);

    /**
     * @brief Copy constructor.
     * @param other The FloatIndexVector to copy from.
     * @throws std::invalid_argument if the input vector in the other object is empty.
     */
    FloatIndexVector(const FloatIndexVector& other);

    /**
     * @brief Copy assignment operator.
     * @param other The FloatIndexVector to copy from.
     * @return Reference to the assigned FloatIndexVector.
     */
    FloatIndexVector& operator=(const FloatIndexVector& other);

    /**
     * @brief Equality operator.
     * @param other The FloatIndexVector to compare with.
     * @return True if the vectors are equal, false otherwise.
     */
    bool operator==(const FloatIndexVector& other) const;

    /**
     * @brief Inequality operator.
     * @param other The FloatIndexVector to compare with.
     * @return True if the vectors are not equal, false otherwise.
     */
    bool operator!=(const FloatIndexVector& other) const;

    /**
     * @brief Sets the hash precision.
     * @param hashPrecision The precision to use for hashing.
     * @throws std::runtime_error if the object is already initialized.
     * @throws std::invalid_argument if hashPrescision is not a positive integer or is >= 14.
     */
    void setHashPrecision(int hashPrecision);

    /**
     * @brief Gets the current hash precision.
     * @return The current hash precision.
     * @throws std::runtime_error if the object is not initialized.
     */
    int getHashPrecision() const;

    /**
     * @brief Sets the vector of floating-point values.
     * @param vec The vector to set.
     * @throws std::runtime_error if the object is already initialized.
     * @throws std::invalid_argument if the input vector is empty.
     */
    void setVector(const std::vector<double>& vec);

    /**
     * @brief Gets the current vector of floating-point values.
     * @return The current vector.
     * @throws std::runtime_error if the object is not initialized.
     */
    std::vector<double> getVector() const;

    /**
     * @brief Initializes the vector with precision.
     * @param vec The vector of floating-point values to initialize with.
     * @param hashPrecision The precision to use for hashing.
     * @throws std::runtime_error if the object is already initialized.
     * @throws std::invalid_argument if the input vector is empty.
     * @throws std::invalid_argument if hashPrescision is not a positive integer or is >= 14.
     */
    void initialize(const std::vector<double>& vec, int hashPrecision);

    /**
     * @brief Initializes the vector without precision.
     * @param vec The vector of floating-point values to initialize with.
     * @throws std::runtime_error if the object is already initialized.
     * @throws std::invalid_argument if the input vector is empty.
     */
    void initialize(const std::vector<double>& vec);

    /**
     * @brief Computes the hash of the vector.
     * @return The hash value of the vector.
     * @throws std::runtime_error if the object is not initialized.
     */
    size_t hash() const;

    /**
     * @brief Reserves memory for the vector and its internal representation.
     * @param size The number of elements to reserve.
     */
    void reserve(size_t size) {
        m_vector.reserve(size);
        m_vectorInt.reserve(size);
    }

    /**
     * @brief Get the size of the index vector
     * @return size The dimension / size of the index vector
     */
    int size() const;

    /**
     * @brief Access element at a specific index.
     * @param index The index of the element to access.
     * @return The value at the specified index.
     * @throws std::out_of_range if the index is out of bounds.
     */
    double operator[](const size_t index) const;

    /**
     * @brief Overloads the << operator for ostream to print FloatIndexVector.
     * @param os The output stream.
     * @param vec The FloatIndexVector to print.
     * @return Reference to the output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const FloatIndexVector& vec);

private:
    /**
     * @brief Sets up the internal representations of the vector and precision.
     *
     * This method initializes the internal vector representations (`m_vector` and `m_vectorInt`)
     * based on the provided floating-point vector and hash precision. It converts the floating-point
     * values into integer representations for consistent hashing.
     *
     * @param vec The vector of floating-point values to initialize with.
     * @param hashPrecision The precision to use for hashing. Values are rounded to this precision.
     * @throws std::invalid_argument if the input vector is empty.
     * @throws std::invalid_argument if hashPrescision is not a positive integer or is >= 14.
     *
     * Example Usage:
     * @code
     * std::vector<double> vec = {1.2345, 2.3456, 3.4567};
     * FloatIndexVector index;
     * index.setupVecs(vec, 2); // Sets up the vector with precision of 2 decimal places
     * @endcode
     */
    void setupVecs(const std::vector<double>& vec, int hashPrecision);

    std::vector<double> m_vector; ///< The vector of floating-point values.
    std::vector<uint64_t> m_vectorInt; ///< Internal representation of the vector for hashing, storing scaled integer values.
    int m_hashPrescision; ///< The precision (number of decimal places) used for rounding and hashing.
    bool m_initialized = false; ///< Flag indicating whether the vector and precision have been initialized.
};

/**
 * @namespace std
 * @brief Specialization of std::hash for FloatIndexVector.
 *
 * This specialization allows FloatIndexVector to be used as a key in unordered containers.
 */
namespace std {
    template <>
    struct hash<FloatIndexVector> {
        /**
         * @brief Computes the hash of a FloatIndexVector.
         * @param vec The FloatIndexVector to hash.
         * @return The hash value of the vector.
         */
        size_t operator()(const FloatIndexVector& vec) const {
            size_t hashValue = vec.hash();
            return hashValue;
        }
    };
}
