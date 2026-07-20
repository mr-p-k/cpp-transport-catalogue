#define _USE_MATH_DEFINES

#include "geo.h"

#include <cmath>

namespace geo {

bool Coordinates::operator==(const Coordinates& other) const {
    return lat == other.lat && lng == other.lng;
}

bool Coordinates::operator!=(const Coordinates& other) const {
    return !(*this == other);
}

double ComputeDistance(Coordinates from, Coordinates to) {
    if (from == to) {
        return 0;
    }

    static const double kDR = M_PI / 180.;
    static const int kEarthRadius = 6371000;

    using namespace std;
    return acos((sin(from.lat * kDR) * sin(to.lat * kDR)) +
                (cos(from.lat * kDR) * cos(to.lat * kDR) *
                 cos(abs(from.lng - to.lng) * kDR))) *
           kEarthRadius;
}

}  // namespace geo