#include "WalkGraph.h"
#include "../FrogFish.h"
#include <BWAPI.h>
#include <BWEM/bwem.h>
#include <jps.hh>

// Leverages BWEM (include/BWEM) and a borrowed JPS implementation (jps.hh)

WalkGraph::WalkGraph() {}

void WalkGraph::init() {
    walk_size = the_map.WalkSize();
    width = walk_size.x;
    height = walk_size.y;
}

std::vector<BWAPI::Position> WalkGraph::get_path(
    const BWAPI::Position &_a, 
    const BWAPI::Position &_b
) {
    BWAPI::WalkPosition a(_a.x / 8, _a.y / 8);
    BWAPI::WalkPosition b(_b.x / 8, _b.y / 8);
    printf("a %s\n", (a.isValid() ? "is valid" : "is not valid"));
    printf("b %s\n", (b.isValid() ? "is valid" : "is not valid"));
    printf("%d, %d\n", a.x, a.y);
    printf("%d, %d\n", b.x, b.y);
    printf("width: %d, height:%d\n", width, height);

    bool found = JPS::findPath(path, the_map, a.x, a.y, b.x, b.y, step);
    printf("%s\n", (found ? "found! :D" : "not found :("));
    std::vector<BWAPI::Position> pos_path;
    if (found) {
        for (auto &walk_pos : path) {
            pos_path.push_back(BWAPI::Position(walk_pos.x * 8, walk_pos.y * 8));
        }
    }
    return pos_path;
}

// in frames
double WalkGraph::get_approx_path_time(
    std::vector<BWAPI::Position> &_path, 
    BWAPI::Position start, 
    int speed
) {
    assert(_path.size() > 0);
    double speed_factor = 1 / (double)speed;
    double frame_time = start.getApproxDistance(_path[0]) * speed_factor;
    for (unsigned int i = 0; i < path.size() - 1; ++i) {
        frame_time += _path[i].getApproxDistance(_path[i + 1]) * speed_factor;
    }
    return frame_time;
}