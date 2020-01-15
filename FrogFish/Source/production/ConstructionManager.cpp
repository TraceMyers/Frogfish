#include "ConstructionManager.h"
#include "BuildOrder.h"
#include "EconTracker.h"
#include "BuildPlacement.h"
#include "../unitdata/BaseStorage.h"
#include "../unitdata/FrogUnit.h"
#include <BWAPI.h>

// for now, just follows the build order and just one instruction at a time
// to build anywhere with any unit at the supply of the build item

void ConstructionManager::take_build_order(BuildOrder *_build_order) {
    build_order = _build_order;
}
void ConstructionManager::build_structures(EconTracker &econ_tracker, BaseStorage &base_storage) {
    BuildItem &build_item = build_order->peek_next();
    if (!building 
        && build_item.build_type == BuildItem::BUILD 
        && build_item.supply_target == Broodwar->self()->supplyUsed()
    ) {
        bool found_drone = false;
        bool found_tilepos = false;
        BWAPI::UnitType making_type = build_item.make_type;
        for (auto &base : base_storage.get_self_bases()) {
            if (!found_tilepos) {
                BWAPI::TilePosition candidate;
                if (making_type == BWAPI::UnitTypes::Zerg_Extractor) {
                    auto &geysers = BuildPlacement::get_base_geysers(base);
                    if (geysers.size() > 0) {
                        candidate = geysers[0]->TopLeft();
                    }
                }
                else {
                    candidate = BuildPlacement::find_any_node_for_placement(
                        base, making_type.tileWidth(), making_type.tileHeight()
                    );
                }
                if (candidate.isValid()) {
                    build_tilepositions.push_back(candidate);
                    found_tilepos = true;
                }
            }
            for (auto &worker : base->get_workers()) {
                if (!found_drone 
                    && worker->is_ready() 
                    && worker->f_task == FrogUnit::MINE_MINERALS
                ) {
                    build_drones.push_back(worker);
                    found_drone = true;
                    break;
                }
            }
            if (found_drone && found_tilepos) {
                break;
            }
        }
        if (!(found_drone && found_tilepos)) {
            if (found_drone) {
                build_tilepositions.erase(build_tilepositions.begin());
            }
            else {
                build_drones.erase(build_drones.begin());
            }
        }
        else {
            building_init = true;
            building = true;
        }
    }
    if (building_init) {
        build_drones[0]->bwapi_u()->move(BWAPI::Position(build_tilepositions[0]));
        build_drones[0]->set_cmd_delay(3);
        building_init = false;
    }
    else if (building) {
        if (build_drones[0]->is_ready()) {
            if (build_drones[0]->get_type() != BWAPI::UnitTypes::Zerg_Drone) {
                building = false;
                build_drones.erase(build_drones.begin());
                build_tilepositions.erase(build_tilepositions.begin());
                build_order->next();
            }
            else {
                BWAPI::Position drone_pos = build_drones[0]->get_pos();
                BWAPI::Position build_pos = Position(build_tilepositions[0]);
                if (drone_pos.getDistance(build_pos) < 256) {
                    build_drones[0]->bwapi_u()->build(
                        build_order->peek_next().make_type,
                        build_tilepositions[0] 
                    );
                    printf("trying to build a %s\n", build_order->peek_next().make_type.getName().c_str());
                    build_drones[0]->set_cmd_delay(7);
                }
            }
        }
    }
}