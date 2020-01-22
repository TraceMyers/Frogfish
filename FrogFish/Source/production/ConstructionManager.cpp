#include "ConstructionManager.h"
#include "BuildOrder.h"
#include "EconTracker.h"
#include "BuildPlacement.h"
#include "../pathing/PathFinding.h"
#include "../unitdata/BaseStorage.h"
#include "../unitdata/UnitStorage.h"
#include "../unitdata/FrogUnit.h"
#include <BWEB/BWEB.h>
#include <BWAPI.h>
#include <assert.h>

// TODO: currently just statically takes econ timing estimates
// right when it takes the build order. Rather, it should take
// the build order then update departure times regularly as
// new estimates come in

// TOOD: once a few things sorted out, total refactor (still)

void ConstructionManager::on_frame_update(
    BaseStorage &base_storage,
    UnitStorage &unit_storage,
    EconTracker &econ_tracker
) {
    construction_storage.on_frame_update(unit_storage, econ_tracker);
}

// assumes items are being built in-base
// in the future, decisions will have to be made about where they're being built 
void ConstructionManager::init_builds(
    BaseStorage &base_storage,
    BuildOrder *build_order,
    EconTracker &econ_tracker,
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
                if (pair[0] == i) {
                    found_pair = true;
                    // for now, any base, any drone
                    bool found_worker = false;
                    for (auto &base : base_storage.get_self_bases()) {
                        for (auto &worker : base->get_workers()) {
                            if (worker->f_task == FrogUnit::MINE_MINERALS) {
                                found_worker = true;
                                BuildItem &item = build_order->get(i);
                                TilePosition build_tp;
                                BWEB::Path path;
                                if (item.make_type == BWAPI::UnitTypes::Zerg_Hatchery) {
                                    build_tp = BuildPlacement::find_expansion_tilepos(
                                        base_storage, 
                                        base
                                    );
                                    path.createUnitPath(
                                        worker->get_pos(),
                                        BWAPI::Position(build_tp)
                                    );
                                }
                                else if (item.make_type == BWAPI::UnitTypes::Zerg_Extractor) {
                                    build_tp = BuildPlacement::get_base_geyser_tilepos(base);
                                    BWAPI::TilePosition path_tp = 
                                        BuildPlacement::get_pathable_geyser_tilepos(build_tp);
                                    path.createUnitPath(
                                        worker->get_pos(),
                                        BWAPI::Position(path_tp)
                                    );
                                }
                                else {
                                    build_tp = BuildPlacement::find_any_node_for_placement(
                                        base, item.make_type.tileWidth(), item.make_type.tileHeight()
                                    );
                                    path.createUnitPath(
                                        worker->get_pos(),
                                        BWAPI::Position(build_tp)
                                    );
                                }
                                
                                if (path.isReachable()) {
                                    int travel_time = 
                                        path.getDistance() / BWAPI::UnitTypes::Zerg_Drone.topSpeed();
                                    if (pair[1] * 24 - travel_time <= 0) {
                                        int reservation_ID = econ_tracker.make_reservation(
                                            item.make_type.mineralPrice(),
                                            item.make_type.gasPrice(),
                                            travel_time * 3
                                        );
                                        construction_storage.add_tracker(
                                            worker,
                                            item.make_type,
                                            build_tp,
                                            path,
                                            i,
                                            reservation_ID
                                        );

                                        worker->f_task = FrogUnit::BUILD_STRUCT;
                                        break;
                                    }
                                }
                                else {
                                    // panic
                                    printf("path not reachable for: %s\n", item.make_type.c_str());
                                }
                            }
                        }
                        if (found_worker) {break;}
                    }
                    break;
                }
                if (found_pair) {break;}
            }
        } 
        else if (
            i == build_order->cur_item 
            && build_order->get(i).build_type == BuildItem::CANCEL
            && build_order->get(i).supply_target == Broodwar->self()->supplyUsed()
        ) {
            // currently only handles extractor?
            int cancel_id = build_order->get(i).required_i;
            if (construction_storage.get_status(cancel_id) == construction_storage.UNDER_CONSTR) {
                FUnit cancel_unit = construction_storage.get_unit(cancel_id);
                cancel_unit->bwapi_u()->cancelConstruction();
                if (cancel_unit->get_type() == BWAPI::UnitTypes::Zerg_Extractor){
                    base_storage.immediately_remove_struct_from_all_bases(cancel_unit);
                }
                build_order->next();
            }
        }
        else if (i == build_order->cur_item && construction_storage.get_unit(i) != nullptr) {
            build_order->next();
        }
    }
}
