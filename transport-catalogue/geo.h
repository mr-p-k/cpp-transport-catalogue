#pragma once

#include <cmath>

namespace geo {

struct Coordinates {
    double lat;
    double lng;
    bool operator==(const Coordinates& other) const {
        return lat == other.lat && lng == other.lng;
    }
    bool operator!=(const Coordinates& other) const {
        return !(*this == other);
    }
};

inline double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    if (from == to) {
        return 0;
    }
    static const double kDR = 3.1415926535 / 180.;
    static const int kEarthRadius = 6371000;
    return acos((sin(from.lat * kDR) * sin(to.lat * kDR)) +
                (cos(from.lat * kDR) * cos(to.lat * kDR) *
                 cos(abs(from.lng - to.lng) * kDR))) *
           kEarthRadius;
}

}  // namespace geo