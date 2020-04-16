#include "Construction.h"
#include "BuildOrder.h"
#include "Economy.h"
#include "BuildGraph.h"
#include "../movement/Move.h"
#include "../basic/Bases.h"
#include "../basic/Units.h"
#include <BWEB/BWEB.h>
#include <BWAPI.h>
#include <assert.h>

// TODO: currently just statically takes econ timing estimates
// right when it takes the build order. Rather, it should take
// the build order then update departure times regularly as
// new estimates come in

// TOOD: once a few things sorted out, total refactor (still)

namespace Production::Construction {
    namespace {
        enum STATUS {
            WAITING,
            MOVING,
            SITE,
            CONSTRUCTION,
            COMPLETED,
            PATH_ERROR,
            DEAD
        };

        std::vector<BWAPI::Unit>                builders;
        std::vector<int>                        build_order_IDs;
        std::vector<int>                        move_IDs;
        std::vector<STATUS>                     statuses;

        bool already_building(int build_ID) {
            for (auto &ID : build_order_IDs) {
                if (ID == build_ID) {return true;}
            }
            return false;
        }
    }

    void on_frame_update() {
        // ?
    }

// TODO:
// - make smart decisions about where to build
// - make smart decisions about which worker to build with
// - cache and check workers to be used
// - cache and check build locations
//      - request scouts for locations out of vision
// - replace cache items if situation significantly changes
// - check against cache for new build locations & workers to use
// - maybe take out resource reservations; instead:
//      - ideally, worker leaves with just enough time to get to site
//      - once at site, money is spent, and the build order is advanced from here
//      - (since all production now goes into the build order, there doesn't seem to be any reason
//         to reserve resources just yet. Reserving money for units/structs that MAY be lost soon 
//         seems like a good idea. That way, the build order can remain the same, and money can
//         just be temporarily witheld for emergency spending.)
void init_builds() {
    auto &econ_sim_data = Economy::sim_data();
    auto &bases = Basic::Bases::self_bases();
    for (unsigned i = BuildOrder::current_index(); i < BuildOrder::size(); ++i) {
        const BuildOrder::Item &build_item = BuildOrder::get(i);
		if (build_item.action == BuildOrder::Item::BUILD && !already_building(i)) {
            for (auto &ID_and_start_time : econ_sim_data) {
                int build_ID = ID_and_start_time[0];
                if (build_ID == i) {
                    for (auto &base : bases) {
                        auto &workers = Basic::Bases::workers(base);
                        for (auto &worker : workers) {
                            auto &worker_data = Basic::Units::data(worker);
                            if (worker_data.u_task == Basic::Refs::MINERALS) {
                                int start_time = ID_and_start_time[1];
                                TilePosition build_tp;
                                if (build_item.unit_type == BWAPI::UnitTypes::Zerg_Hatchery) {
                                    // TODO: handle hatchery building/placement
                                }
                                else if (build_item.unit_type == BWAPI::UnitTypes::Zerg_Extractor) {
                                    build_tp = BuildGraph::get_geyser_tilepos(base);
                                }
                                else {
                                    const BWAPI::UnitType &build_type = build_item.unit_type;
                                    build_tp = BuildGraph::get_build_tilepos(
                                        base, 
                                        build_type.tileWidth(),
                                        build_type.tileHeight()
                                    );
                                }
                                int move_ID = Movement::Move::move(worker, build_tp);
                                if (move_ID >= 0) {

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

}