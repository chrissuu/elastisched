#ifndef INTERVAL_H
#define INTERVAL_H

#include <stdexcept>  // for std::invalid_argument

template<typename T>
class Interval {
private:
    T low_;
    T high_;

public:
    // Constructor
    Interval(T low, T high) : low_(low), high_(high) {
        if (high < low) {
            throw std::invalid_argument("Interval: high must be >= low");
        }
    }

    // Accessors
    T low() const { return low_; }
    T high() const { return high_; }

    // Equality operators
    bool operator==(const Interval& other) const {
        return low_ == other.low_ && high_ == other.high_;
    }

    bool operator!=(const Interval& other) const {
        return !(*this == other);
    }

    // Check if intervals overlap
    bool overlaps(const Interval& other) const {
        return !(high_ < other.low_ || other.high_ < low_);
    }

    // Check if contains a value
    bool contains(T value) const {
        return low_ <= value && value <= high_;
    }

    // Length of interval
    T length() const {
        return high_ - low_;
    }
};

#endif // INTERVAL_H
