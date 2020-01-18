#pragma once
#pragma message("including PathFinding")

#include "../unitdata/FrogUnit.h"
#include <jps.hh>
#include <BWAPI.h>
#include <BWEM/bwem.h>
#include <vector>

namespace PathFinding {
    

    void init();
    bool move(FUnit unit, std::vector<BWAPI::Position> &path, int close_enough);
    bool attack_move(FUnit unit, std::vector<BWAPI::Position> &path, int close_enough);
    double get_approx_path_time(
        std::vector<BWAPI::Position> &_path, 
        BWAPI::Position start, 
        int speed
    );
    std::vector<BWAPI::Position> get_path(
        const BWAPI::Position &_a, 
        const BWAPI::Position &_b
    );
    std::vector<BWAPI::Position> get_path_near(
        const BWAPI::Position &_a, 
        const BWAPI::Position &_b
    );
}