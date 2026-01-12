#ifndef INTERVAL_H
#define INTERVAL_H

#include <algorithm>
#include <iostream>
#include <stdexcept>

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
    Interval(T e) : low(e), high(e) {}

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
        if (low == high) {
            return other.getLow() <= low && low < other.getHigh();
        }
        if (other.getLow() == other.getHigh()) {
            return low <= other.getLow() && other.getLow() < high;
        }
        return !(high <= other.getLow() || other.getHigh() <= low);
    }

    bool contains(const Interval& other) const {
        return low <= other.getLow() && other.getHigh() <= high;
    }

    T overlap_length(const Interval& other) const {
        if (!this->overlaps(other)) return 0;
        const T start = std::max(low, other.getLow());
        const T end = std::min(high, other.getHigh());
        return end > start ? end - start : 0;
    }

    T length() const {
        return high - low;
    }
};

#endif // INTERVAL_H
