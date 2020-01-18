#include "ConstructionManager.h"
#include "BuildOrder.h"
#include "EconTracker.h"
#include "BuildPlacement.h"
#include "../pathing/PathFinding.h"
#include "../unitdata/BaseStorage.h"
#include "../unitdata/UnitStorage.h"
#include "../unitdata/FrogUnit.h"
#include <BWAPI.h>
#include <assert.h>

// TODO: currently just statically takes econ timing estimates
// right when it takes the build order. Rather, it should take
// the build order then update departure times regularly as
// new estimates come in

// behavior : 2 drones sent to make geyser before 1 drone sent to make pool
// didn't have enough for pool

// TOOD: once a few things sorted out, total refactor (still)

void ConstructionManager::on_frame_update(
    BaseStorage &base_storage,
    UnitStorage &unit_storage
) {
    construction_storage.on_frame_update(unit_storage);
}

// assumes items are being built in-base
// in the future, decisions will have to be made about where they're being built 
void ConstructionManager::init_builds(
    BaseStorage &base_storage,
    BuildOrder *build_order,
    std::vector<std::vector<int>> &econ_time_est
) {
    for (
        unsigned i = build_order->cur_item; 
        i < build_order->size();
        ++i
    ) {
        if (
            build_order->get(i).build_type == BuildItem::BUILD 
            && construction_storage.get_unit(i) == nullptr
        ) {
            bool found_pair = false;
            for (auto &pair : econ_time_est) {
                if (pair[0] == i && pair[1] * 24 <= IN_BASE_TRAVEL_FRAMES) {
                    found_pair = true;
                    // for now, any base, any drone
                    bool found_worker = false;
                    for (auto &base : base_storage.get_self_bases()) {
                        for (auto &worker : base->get_workers()) {
                            if (worker->f_task == FrogUnit::MINE_MINERALS) {
                                BuildItem &item = build_order->get(i);
                                TilePosition build_tp;
                                std::vector<BWAPI::Position> path;
                                if (item.make_type == BWAPI::UnitTypes::Zerg_Extractor) {
                                    build_tp = BuildPlacement::get_base_geyser_tilepos(base);
                                    path = PathFinding::get_path_near(
                                        worker->get_pos(), BWAPI::Position(build_tp)
                                    );
                                }
                                else {
                                    build_tp = BuildPlacement::find_any_node_for_placement(
                                        base, item.make_type.tileWidth(), item.make_type.tileHeight()
                                    );
                                    path = PathFinding::get_path(
                                        worker->get_pos(), BWAPI::Position(build_tp)
                                    );
                                }

                                construction_storage.add_tracker(
                                    worker,
                                    item.make_type,
                                    build_tp,
                                    i,
                                    path
                                );

                                worker->f_task = FrogUnit::BUILD_STRUCT;
                                found_worker = true;
                                break;
                            }
                        }
                        if (found_worker) {break;}
                    }
                    break;
                }
                if (found_pair) {break;}
            }
            
        } 
        else if (i == build_order->cur_item && construction_storage.get_unit(i) != nullptr) {
            build_order->next();
        }
    }
}