#ifndef INTERVAL_H
#define INTERVAL_H

#include <stdexcept>  // for std::invalid_argument

template<typename T>
class Interval {
private:
    T low;
    T high;

public:
    // Constructor
    Interval(T time) : low(time), high(time) {}

    Interval(T low, T high) : low(low), high(high) {
        if (high < low) {
            throw std::invalid_argument("Interval: high must be >= low");
        }
    }

    // Accessors
    T getLow() const { return low; }
    T getHigh() const { return high; }

    // Equality operators
    bool operator==(const Interval& other) const {
        return low == other.getLow() && high == other.getHigh();
    }

    bool operator!=(const Interval& other) const {
        return !(*this == other);
    }

    // Check if intervals overlap
    bool overlaps(const Interval& other) const {
        return !(high < other.getLow() || other.getHigh() < low);
    }

    // Check if contains a value
    bool contains(T value) const {
        return low <= value && value <= high;
    }

    // Length of interval
    T length() const {
        return high - low;
    }
};

#endif // INTERVAL_H
