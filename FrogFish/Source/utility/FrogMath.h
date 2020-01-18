#pragma once
#pragma message("including FrogMath")

#include <BWAPI.h>
#include "../unitdata/FrogBase.h"
#include "../unitdata/FrogUnit.h"

namespace FrogMath {

    const double FM_PI = 3.14159265359;
    
    double get_angle(BWAPI::Position a, BWAPI::Position b);
    bool angle_is_between(double check, double a, double b, double inside);
    bool pos_is_between_angle(
        BWAPI::Position from_pos,
        BWAPI::Position check, 
        double a_angle, 
        double b_angle,
        double inside_angle
    );
    std::vector<double> unit_vector(BWAPI::Position a, BWAPI::Position b);
    std::vector<double> get_buffered_resource_angles(FBase base);
}