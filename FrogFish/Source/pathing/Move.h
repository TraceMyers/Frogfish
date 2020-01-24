#pragma once
#pragma message("including PathFinding")

#include "../unitdata/FrogUnit.h"
#include <BWAPI.h>
#include <BWEB/BWEB.h>

namespace Pathing {
    void init();
    bool move(FUnit unit, BWEB::Path &path, int close_enough);
    bool attack_move(FUnit unit, BWEB::Path &path, int close_enough);
}