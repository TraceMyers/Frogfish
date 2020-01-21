#pragma once
#pragma message("including PathFinding")

#include "../unitdata/FrogUnit.h"
#include <jps.hh>
#include <BWAPI.h>
#include <BWEM/bwem.h>
#include <vector>

namespace PathFinding {
    void init();
    bool move(FUnit unit, BWEB::Path &path, int close_enough);
    bool attack_move(FUnit unit, BWEB::Path &path, int close_enough);
}