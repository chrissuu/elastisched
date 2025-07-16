#include <iostream>

template<typename T>
class Interval {
private:
    T low_;
    T high_;

public:
    // Constructor
    Interval(T low, T high) : low_(low), high_(high) {
        if (high < low) {
            throw std::invalid_argument("high must be >= low");
        }
    }

    // Accessors
    T low() const { return low_; }
    T high() const { return high_; }

    // Equality operator
    bool operator==(const Interval& other) const {
        return low_ == other.low_ && high_ == other.high_;
    }

    // Inequality operator
    bool operator!=(const Interval& other) const {
        return !(*this == other);
    }

    // Check if this interval overlaps with another
    bool overlaps(const Interval& other) const {
        // Overlap if they share any point
        return !(high_ < other.low_ || other.high_ < low_);
    }

    // Check if this interval contains a value
    bool contains(T value) const {
        return low_ <= value && value <= high_;
    }

    // Length of the interval (assuming T supports subtraction)
    T length() const {
        return high_ - low_;
    }
};
