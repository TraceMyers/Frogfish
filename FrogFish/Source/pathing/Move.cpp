#include "Move.h"
#include "../FrogFish.h"
#include "../unitdata/FrogUnit.h"
#include <BWAPI.h>
#include <vector>
#include <assert.h>

namespace Pathing {

bool move(FUnit unit, BWEB::Path &path, int close_enough) {
    int waypoint = unit->waypoint;
    std::vector<TilePosition> &path_tiles = path.getTiles();
    int path_size = path_tiles.size();
    while (
        waypoint < path_size
        && unit->get_pos().getApproxDistance(BWAPI::Position(path_tiles[waypoint])) 
            < close_enough
    ) {
        ++waypoint;
    }
    unit->waypoint = waypoint;
    if (waypoint < path_size) {
        unit->bwapi_u()->move(BWAPI::Position(path_tiles[waypoint]));
        unit->set_cmd_delay(2);
        return false;
    }
    else {
        return true;
    }
}

bool attack_move(FUnit unit, BWEB::Path &path, int close_enough) {
    int waypoint = unit->waypoint;
    std::vector<TilePosition> &path_tiles = path.getTiles();
    int path_size = path_tiles.size();
    while (
        waypoint < path_size
        && unit->get_pos().getApproxDistance(BWAPI::Position(path_tiles[waypoint])) 
            < close_enough
    ) {
        ++waypoint;
    }
    unit->waypoint = waypoint;
    if (waypoint < path_size) {
        unit->bwapi_u()->attack(BWAPI::Position(path_tiles[waypoint]));
        unit->set_cmd_delay(2);
        return false;
    }
    else {
        return true;
    }
}

}