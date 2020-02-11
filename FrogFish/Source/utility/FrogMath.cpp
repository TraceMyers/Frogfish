#include "FrogMath.h"
#include <BWAPI.h>
#include <cmath>

namespace FrogMath{

// 'a' = from, 'b' = to. Returns angle in radians.
double get_angle(BWAPI::Position a, BWAPI::Position b) {
    double angle = atan2(a.y - b.y, b.x - a.x); 
    if (angle < 0) {
        angle = 2 * PI + angle;
    }
    return angle;
}

// Requires a reference angle for what constitutes "inside", or in-between
// a and b. This allows it to work with angles > 180 degrees
// always returns true if 'check' == a or b
bool angle_is_between(double check, double a, double b, double inside) {
    if (b < a) {
        if (inside > b && inside < a) {
            return check > b && check < a;
        }
        else {
            return (check < b || check > a);
        }
    }
    else {
        if (inside > a && inside < b) {
            return check > a && check < b;
        }
        else {
            return (check < a || check > b);
        }
    }
}

// Requires a reference angle for what constitutes "inside", or in-between
// a and b. This allows it to work with angles > 180 degrees
// always returns true if 'check' == a or b
bool pos_is_between_angle(
    BWAPI::Position from_pos,
    BWAPI::Position check, 
    double a_angle, 
    double b_angle,
    double inside_angle
) {
    double check_angle = get_angle(from_pos, check);

    return angle_is_between(check_angle, a_angle, b_angle, inside_angle);
}

// 'a' = from, 'b' = to.
std::vector<double> unit_vector(BWAPI::Position a, BWAPI::Position b) {
    b.x -= a.x;
    b.y -= a.y;
    double 
        one_over_hyp = 1.0 / sqrt((double)b.x * b.x + (double)b.y * b.y),
        x = (double)b.x * one_over_hyp,
        y = (double)b.y * one_over_hyp;
    std::vector<double> unit_vec;
    unit_vec.push_back(x);
    unit_vec.push_back(y);
    return unit_vec;
}

}