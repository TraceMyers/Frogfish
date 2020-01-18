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

void ConstructionManager::take_build_order(
    BaseStorage &base_storage,
    BuildOrder *_build_order,
    std::vector<std::vector<int>> &econ_timing_estimates
) {
    for (unsigned i = 0; i < build_order->size(); ++i) {
        BuildItem &item = build_order->get(i);
        if (item.build_type == BuildItem::BUILD) {
            init_construction_default(base_storage, econ_timing_estimates, item, (int)i);
        }
    }
}
void ConstructionManager::on_frame_update(
    BaseStorage &base_storage,
    UnitStorage &unit_storage
) {
    construction_storage.on_frame_update(unit_storage);
}

// called for all build tasks (looking ahead in build order)
// TODO: needs to return a value saying it found a worker, keep trying
// until that happens
void ConstructionManager::init_construction_default(
    BaseStorage &base_storage,
    std::vector<std::vector<int>> &econ_timing_estimates,
    BuildItem &item,
    int item_i
) {
    // just get any old drone for now
    bool worker_found = false;
    for (auto &base : base_storage.get_self_bases()) {
        for (auto &worker : base->get_workers()) {
            if (worker->f_task == FrogUnit::MINE_MINERALS) {
                worker_found = true;
                worker->f_task = FrogUnit::BUILD_STRUCT;
                // build at this base
                TilePosition build_tp;
                if (item.make_type == BWAPI::UnitTypes::Resource_Vespene_Geyser) {
                    build_tp = BuildPlacement::get_base_geyser_tilepos(base);
                }
                else {
                    build_tp = BuildPlacement::find_any_node_for_placement(
                        base, item.make_type.tileWidth(), item.make_type.tileHeight()
                    );
                }
                std::vector<BWAPI::Position> path = PathFinding::get_path(
                    worker->get_pos(), BWAPI::Position(build_tp)
                );
                int frame_distance = PathFinding::get_approx_path_time(
                    path,
                    worker->get_pos(),
                    BWAPI::UnitTypes::Zerg_Drone.topSpeed()
                );
                int departure_frames;
                for (unsigned i = 0; i < econ_timing_estimates.size(); ++i) {
                    if (econ_timing_estimates[i][0] == item_i) {
                        departure_frames = 
                            econ_timing_estimates[i][1] * 24 - frame_distance;
                    }
                }
                departure_frames = (departure_frames < 0 ? 0 : departure_frames);
                construction_storage.add_tracker(
                    worker,
                    item.make_type,
                    build_tp,
                    item_i,
                    path,
                    departure_frames
                );
                break;
            }
        }
        if (worker_found) {
            break;
        }
    }
}