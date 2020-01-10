#include "data/FrogBase.h"
#include <BWEM/bwem.h>
#include <BWAPI.h>
#include <vector>

std::vector<BWAPI::TilePosition> temp_grid;

void generate_build_grid(BWEM::Map &the_map, FBase base) {
    for (auto &hatch : base->get_resource_depots()) {
        TilePosition hatch_tilepos = hatch->get_tilepos();
        bool already_checked = false;
        for (auto &tp : temp_grid) {
            if (tp == hatch_tilepos) {
                already_checked = true;
                break;
            }
        }
        if (!already_checked) {
            std::vector<BWAPI::TilePosition> cur_square {
                BWAPI::TilePosition(hatch_tilepos.x - 1, hatch_tilepos.y - 1),
                BWAPI::TilePosition(hatch_tilepos.x, hatch_tilepos.y - 1),
                BWAPI::TilePosition(hatch_tilepos.x + 1, hatch_tilepos.y - 1),
                BWAPI::TilePosition(hatch_tilepos.x + 1, hatch_tilepos.y),
                BWAPI::TilePosition(hatch_tilepos.x + 1, hatch_tilepos.y + 1),
                BWAPI::TilePosition(hatch_tilepos.x, hatch_tilepos.y + 1),
                BWAPI::TilePosition(hatch_tilepos.x - 1, hatch_tilepos.y + 1),
                BWAPI::TilePosition(hatch_tilepos.x - 1, hatch_tilepos.y)
            };
            for (auto tp : cur_square) {
                if (zerg_can_build_on(the_map, tp)) {
                    temp_grid.push_back(tp);
                }
            }
            int corner_step = 2,
            std::vector<BWAPI::TilePosition> next_square;
            while(true) {
                int step = 0;
                float center_step = 
                while (step < cur_square.size()) {
                    if (step % corner_step == 0) {

                    }
                    else {

                    }
                    ++step;
                }
                corner_step += 2;
            }
        }
    }
}

bool zerg_can_build_on(BWEM::Map &the_map, BWAPI::TilePosition &tp) {
    if (
        the_map.Valid(tp) 
        && Broodwar->isBuildable(tp, true) 
        && Broodwar->hasCreep(tp)
    ) {
        return true;
    }
    return false;
}

const std::vector<BWAPI::TilePosition> &get_grid() {
return temp_grid;
}
