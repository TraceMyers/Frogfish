#include "FrogMath.h"
#include <BWAPI.h>
#include <cmath>
#include "../unitdata/FrogBase.h"
#include "../unitdata/FrogUnit.h"


// 'a' = from, 'b' = to. Returns angle in radians.
double FrogMath::get_angle(BWAPI::Position a, BWAPI::Position b) {
    double angle = atan2(a.y - b.y, b.x - a.x); 
    if (angle < 0) {
        angle = 2 * FrogMath::FM_PI + angle;
    }
    return angle;
}

// Requires a reference angle for what constitutes "inside", or in-between
// a and b. This allows it to work with angles > 180 degrees
// always returns true if 'check' == a or b
bool FrogMath::angle_is_between(double check, double a, double b, double inside) {
    bool check_val;
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
    return check_val;
}

// Requires a reference angle for what constitutes "inside", or in-between
// a and b. This allows it to work with angles > 180 degrees
// always returns true if 'check' == a or b
bool FrogMath::pos_is_between_angle(
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
std::vector<double> FrogMath::unit_vector(BWAPI::Position a, BWAPI::Position b) {
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

std::vector<double> FrogMath::get_buffered_resource_angles(FBase base) {
    std::vector<double> return_angles {-1, -1, -1}; 
    auto &minerals = base->get_minerals();
    if (minerals.size() <= 1 || base->get_resource_depot_ct() == 0) {
        return return_angles;
    }
    BWAPI::Position hatch_center = base->get_center();
    hatch_center.x += 64;
    hatch_center.y += 48;
    double max_resource_angle_diff = 0.0;
    BWEM::Mineral *max_dist_mineral;
    std::vector<double> angles;

    auto &geysers = base->get_geysers();
    if (geysers.size() > 0) {
        for (auto & geyser : geysers) {
            if (geyser->Pos().getApproxDistance(hatch_center) < 400) {
                double geyser_angle = FrogMath::get_angle(hatch_center, geyser->Pos());
                angles.push_back(geyser_angle);
            } 
        }
    }
    for (auto &mineral : minerals) {
        printf("%.2lf\n", FrogMath::get_angle(hatch_center, mineral->Pos()));
        if (mineral->Pos().getApproxDistance(hatch_center) < 300) {
            angles.push_back(FrogMath::get_angle(hatch_center, mineral->Pos()));
        }
    }
    for (unsigned int i = 0; i < angles.size() - 1; ++i) {
        for (unsigned int j = i + 1; j < angles.size(); ++j) {
            if (angles[j] < angles[i]) {
                double temp = angles[i];
                angles[i] = angles[j];
                angles[j] = temp;
            }
        }
    }

    double sub_value = 0.0;
    std::vector<double> saved_angles;
    for (double element : angles) {
        saved_angles.push_back(element);
    }
    bool found_gap = false;
    for (unsigned int i = 1; i < angles.size(); ++i) {
        if (!found_gap && angles[i] - angles[i - 1] > 2.8) {
            sub_value = angles[i];
            found_gap = true;
            for (unsigned int j = 0; j < i; ++j) {
                angles[j] += 2 * FrogMath::FM_PI - sub_value;
            }
        }
        if (found_gap) {
            angles[i] -= sub_value;
        }
    }

    int max_i = std::distance(angles.begin(), std::max_element(angles.begin(), angles.end()));
    int min_i = std::distance(angles.begin(), std::min_element(angles.begin(), angles.end()));
    int inside_i = 0;
    for (int i = 0; i < angles.size(); ++i) {
        if (i != max_i && i != min_i) {
            inside_i = i;
            break;
        }
    }
    return_angles[0] = saved_angles[min_i] - 0.4;
    if (return_angles[0] < 0) {
        return_angles[0] += 2 * FrogMath::FM_PI;
    }
    return_angles[1] = saved_angles[max_i] + 0.4;
    if (return_angles[1] > 2 * FrogMath::FM_PI) {
        return_angles[1] -= 2 * FrogMath::FM_PI;
    }
    return_angles[2] = saved_angles[inside_i];
    return return_angles;
}