#ifndef INTERVAL_H
#define INTERVAL_H

#include <stdexcept>
#include <iostream>

template<typename T>
class Interval {
private:
    T low;
    T high;

    friend std::ostream& operator<<(std::ostream& os, const Interval<T>& interval) {
        os << "Low: " << interval.getLow() << " High: " << interval.getHigh();
        return os;
    }

public:
    // Constructor
    Interval(T time) : low(time), high(time) {}

    Interval(T low, T high) : low(low), high(high) {
        if (high < low) {
            throw std::invalid_argument("Interval: high must be >= low");
        }
    }

    T getLow() const { return low; }
    T getHigh() const { return high; }

    bool operator==(const Interval& other) const {
        return low == other.getLow() && high == other.getHigh();
    }

    bool operator!=(const Interval& other) const {
        return !(*this == other);
    }

    bool overlaps(const Interval& other) const {
        return !(high < other.getLow() || other.getHigh() < low);
    }

    bool contains(T value) const {
        return low <= value && value <= high;
    }

    T length() const {
        return high - low;
    }
};

#endif // INTERVAL_H
