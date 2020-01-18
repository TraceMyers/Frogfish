#include "../FrogFish.h"
#include "PathFinding.h"
#include <jps.hh>
#include "../unitdata/FrogUnit.h"
#include <BWAPI.h>
#include <vector>
#include <assert.h>

namespace PathFinding {
	JPS::PathVector path;
	BWAPI::WalkPosition walk_size;
	int width;
	int height;
}

bool PathFinding::move(FUnit unit, std::vector<BWAPI::Position> &path, int close_enough) {
    int path_size = path.size();
    assert(unit != nullptr);
    assert(path_size > 0);
    assert(unit->is_ready());
    int waypoint = unit->waypoint;
    while (
        waypoint < path_size
        && unit->get_pos().getApproxDistance(path[waypoint]) < close_enough
    ) {
        ++waypoint;
    }
    unit->waypoint = waypoint;
    if (waypoint < path_size) {
        unit->bwapi_u()->move(path[waypoint]);
        unit->set_cmd_delay(2);
        return false;
    }
    else {
        return true;
    }
}

bool PathFinding::attack_move(FUnit unit, std::vector<BWAPI::Position> &path, int close_enough) {
    int path_size = path.size();
    assert(unit != nullptr);
    assert(path_size > 0);
    assert(unit->is_ready());
    int waypoint = unit->waypoint;
    while (
        waypoint < path_size
        && unit->get_pos().getApproxDistance(path[waypoint]) < close_enough
    ) {
        ++waypoint;
    }
    unit->waypoint = waypoint;
    if (waypoint < path_size) {
        unit->bwapi_u()->attack(path[waypoint]);
        unit->set_cmd_delay(2);
        return false;
    }
    else {
        return true;
    }
}

double PathFinding::get_approx_path_time(
    std::vector<BWAPI::Position> &_path, 
    BWAPI::Position start, 
    int speed
) {
    assert(_path.size() > 0);
    double speed_factor = 1 / (double)speed;
    double frame_time = start.getApproxDistance(_path[0]) * speed_factor;
    for (unsigned int i = 0; i < _path.size() - 1; ++i) {
        frame_time += _path[i].getApproxDistance(_path[i + 1]) * speed_factor;
    }
    return frame_time;
}

void PathFinding::init() {
    walk_size = the_map.WalkSize();
    width = walk_size.x;
    height = walk_size.y;
}

std::vector<BWAPI::Position> PathFinding::get_path(
    const BWAPI::Position &_a, 
    const BWAPI::Position &_b
) {
    BWAPI::WalkPosition a(_a.x / 8, _a.y / 8);
    BWAPI::WalkPosition b(_b.x / 8, _b.y / 8);

    bool found = JPS::findPath(path, the_map, a.x, a.y, b.x, b.y, 0);

    std::vector<BWAPI::Position> pos_path;
    if (found) {
        for (auto &walk_pos : path) {
            pos_path.push_back(BWAPI::Position(walk_pos.x * 8, walk_pos.y * 8));
        }
    }
    return pos_path;
}

std::vector<BWAPI::Position> PathFinding::get_path_near(
    const BWAPI::Position &_a,
    const BWAPI::Position &_b
) {
    BWAPI::WalkPosition a(_a.x / 8, _a.y / 8);
    BWAPI::WalkPosition b(_b.x / 8, _b.y / 8);
    int dirs[4][2] {{1, 0}, {0, -1}, {-1, 0}, {0, 1}};
    int step_max = 1;
    int step_counter = 0;
    int i = 0;
    bool found = false;

    while (!found && step_max < 12) {
        for ( ; step_max < 12; ) {
            b = BWAPI::WalkPosition(b.x + dirs[i][0], b.y + dirs[i][1]);
            if (b.isValid() && the_map.GetMiniTile(b).Walkable()) {
                break;
            }
            ++step_counter;
            if (step_counter == step_max) {
                step_counter = 0;
                ++step_max;
                ++i;
                if (i > 3) {
                    i = 0;
                }
            }
        }
        found = JPS::findPath(path, the_map, a.x, a.y, b.x, b.y, 0);
    }

    std::vector<BWAPI::Position> pos_path;
    if (found) {
        for (auto &walk_pos : path) {
            pos_path.push_back(BWAPI::Position(walk_pos.x * 8, walk_pos.y * 8));
        }
    }
    return pos_path;
}