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

// TODO:
// - make smart decisions about where to build
// - make smart decisions about which worker to build with
// - periodically check if building is still feasible while status == WAITING
//      - request scouts for locations out of vision
//	- if a move ID is invalid when start() is called, problem

namespace Production::Construction {

    namespace {

        enum STATUS {
            WAITING,
            MOVING,
            AT_SITE,
            GIVEN_BUILD_CMD,
            BUILDING,
            COMPLETED,
            PATH_ERROR,
            DEAD,
            NONE
        };

        std::vector<std::vector<BWAPI::Unit>>   unused_workers;
        std::vector<BWAPI::Unit>                builders;
        std::vector<const BWEM::Base *>         build_bases;
        std::vector<BWAPI::TilePosition>        build_locations;
        std::vector<int>                        build_order_IDs;
        std::vector<int>                        move_IDs;
        std::vector<STATUS>                     statuses;
        std::vector<BWAPI::Unit>                canceled_builders;
        BWTimer									check_timer;
        const int                               FINISH_BUILD_FRAMES = 30;
        const int                               BUILD_CMD_DELAY = 2;
		const int								CHECK_INTERVAL = 5;

        bool already_cached(int build_ID) {
            for (auto &ID : build_order_IDs) {
                if (ID == build_ID) {return true;}
            }
            return false;
        }

        void update_unused_workers() {
            auto &bases = Basic::Bases::self_bases();
            for (unsigned i = 0; i < bases.size(); ++i) {
                auto &base = bases[i];
                auto &base_unused_workers = unused_workers[i];
                base_unused_workers = Basic::Bases::workers(base);
                for (
                    auto worker_it = base_unused_workers.begin(); 
                    worker_it != base_unused_workers.end(); 
                    ++worker_it
                ) {
                    auto &worker = *worker_it;
                    auto &worker_data = Basic::Units::data(worker);
                    // TODO: make better decision about whether or not the worker is fit to build
                    if (worker_data.u_task != MINERALS) {
                        worker_it = base_unused_workers.erase(worker_it);
                    }
                    else for (auto &builder : builders) {
                        if (builder == *worker_it) {
                            worker_it = base_unused_workers.erase(worker_it);
                            break;
                        }
                    }
					if (worker_it == base_unused_workers.end()) {break;}
                }
            }
        }

        const std::vector<int> *get_relevant_sim_data(
            int build_ID, 
            const std::vector<std::vector<int>> &econ_sim_data
        ) {
            for (auto &data_item : econ_sim_data) {
                if (data_item[0] == build_ID) {
                    return &data_item;
                }
            }
            return nullptr;
        }

        // temp process, temp params (just creating good func names and stand-in processes)
        int select_base_for_construction(const std::vector<const BWEM::Base *> &bases) {
            for (unsigned j = 0; j < bases.size(); ++j) {
                auto &base_unused_workers = unused_workers[j];
                int base_unused_workers_size = base_unused_workers.size();
                if (base_unused_workers_size > 0) {
                    return j;
                }
            }
			return -1;
        }

        // temp process, temp params (just creating good func names and stand-in processes)
        const BWAPI::Unit select_unused_worker(int base_index) {
            auto &base_unused_workers = unused_workers[base_index];
            auto &builder = base_unused_workers[base_unused_workers.size() - 1];
            base_unused_workers.erase(base_unused_workers.end() - 1);
            return builder;
        }

        void remove_build(int constr_ID) {
            if (statuses[constr_ID] == BUILDING) {
                builders[constr_ID]->cancelConstruction();
                // important for extractors. can't wait until it's done automatically next frame
                // or get read access vioation
                Basic::Bases::remove_struct_from_owner_base(builders[constr_ID]);
            }
            else if (statuses[constr_ID] == GIVEN_BUILD_CMD)   {builders[constr_ID]->stop();}
            else if (statuses[constr_ID] == MOVING)            {Movement::Move::remove(move_IDs[constr_ID]);}
            builders.erase(builders.begin() + constr_ID);
            build_bases.erase(build_bases.begin() + constr_ID);
            build_locations.erase(build_locations.begin() + constr_ID);
            build_order_IDs.erase(build_order_IDs.begin() + constr_ID);
            move_IDs.erase(move_IDs.begin() + constr_ID);
            statuses.erase(statuses.begin() + constr_ID);
        }

        void resolve_dead_builders() {
            // exists()? does it even work?

            for (unsigned i = 0; i < statuses.size(); ++i) {
                if (statuses[i] == DEAD) {
                    remove_build(i);
                    --i;
                }
            }
            auto &dead_units = Basic::Units::self_just_destroyed();
            for (int i = 0; i < dead_units.size(); ++i) {
                const BWAPI::Unit &dead_unit = dead_units[i];
                for (unsigned j = 0; j < builders.size(); ++j) {
                    auto &builder = builders[j];
                    if (builder == dead_unit) {
                        statuses[j] = DEAD;
                        break;
                    }
                }
            }
        }

        void init_builds() {
            auto &econ_sim_data = Economy::sim_data();
            bool updated_unused_workers = false;
            for (unsigned build_ID = BuildOrder::current_index(); build_ID < BuildOrder::size(); ++build_ID) {
                const BuildOrder::Item &build_item = BuildOrder::get(build_ID);
                const std::vector<int> *item_sim_data_ptr = get_relevant_sim_data(build_ID, econ_sim_data);
                if (
                    build_item.action() == BuildOrder::Item::BUILD 
                    && !already_cached(build_ID) 
                    && item_sim_data_ptr != nullptr
                ) {
					if (!updated_unused_workers) {
                        update_unused_workers();
                        updated_unused_workers = true;
                    }
                    auto &bases = Basic::Bases::self_bases();
                    int base_index = select_base_for_construction(bases);
					if (base_index < 0) continue;
                    auto &base = bases[base_index];
                    BWAPI::Unit builder = select_unused_worker(base_index);
                    BWAPI::TilePosition build_tp;
                    //if (build_item.unit_type() == BWAPI::UnitTypes::Zerg_Hatchery) {
                        // TODO: handle hatchery building/placement
                    //}
                    if (build_item.unit_type() == BWAPI::UnitTypes::Zerg_Extractor) {
                        build_tp = BuildGraph::get_geyser_tilepos(base);
                    }
                    else {
                        const BWAPI::UnitType &build_type = build_item.unit_type();
                        build_tp = BuildGraph::get_build_tilepos(
                            base, 
                            build_type.tileWidth(),
                            build_type.tileHeight()
                        );
                    }
                    int move_ID = Movement::Move::move(builder, build_tp, false, true);
                    if (move_ID < 0) {
                        // TODO: pathing error! need to deal with this somehow
                        continue;
                    }

                    builders.push_back(builder);
                    build_bases.push_back(base);
                    build_locations.push_back(build_tp);
                    build_order_IDs.push_back(build_ID);
                    move_IDs.push_back(move_ID);
                    statuses.push_back(WAITING);
					printf("start\n");
					printf("%d, %d\n", build_tp.x, build_tp.y);
					printf("%d\n", build_ID);
					printf("%d\n", move_ID);
					printf("end\n");
                } 
            }
        }

        void advance_builds() {
            unsigned build_ID = BuildOrder::current_index();
            auto &cur_build_item = BuildOrder::current_item();
            int cur_constr_ID = -1;
            for (unsigned i = 0; i < build_order_IDs.size(); ++i) {
                if (build_order_IDs[i] == build_ID) {
                    cur_constr_ID = i;
                    break;
                }
            }
            if (cur_build_item.action() == BuildOrder::Item::CANCEL) {
                int cancel_build_ID = cur_build_item.cancel_index();
                for (unsigned i = 0; i < build_order_IDs.size(); ++i) {
                    if (build_order_IDs[i] == cancel_build_ID) {
                        canceled_builders.push_back(builders[i]);
                        remove_build(i);
                        break;
                    }
                }
                // Advance the build order even if it's too late to cancel
                BuildOrder::next();
            }
            else if (cur_constr_ID >= 0) {
                if (statuses[cur_constr_ID] == WAITING) {
                    int move_ID = move_IDs[cur_constr_ID];
                    int travel_time = Movement::Move::remaining_frames(move_ID);
					printf("1\n");
                    const std::vector<int> *item_sim_data_ptr = get_relevant_sim_data(
                        build_ID, 
                        Economy::sim_data()
                    );
                    if (item_sim_data_ptr != nullptr) {
						printf("2\n");
                        int start_build_frames = (*item_sim_data_ptr)[1] * 24;
                        if (travel_time <= start_build_frames) {
							printf("3\n");
                            Movement::Move::start(move_ID);
                            BWAPI::Unit builder = builders[cur_constr_ID];
                            Basic::Units::set_utask(builder, Basic::Refs::BUILD);
                            statuses[cur_constr_ID] = MOVING;
                        }
                    }
                    else {
                        // TODO: address error
                    }
                }
                else if (statuses[cur_constr_ID] == MOVING) {
					printf("moving\n");
                    int move_ID = move_IDs[cur_constr_ID];
                    // TODO: account for potential move errors
                    if (Movement::Move::get_status(move_ID) == Movement::Move::DESTINATION) {
                        statuses[cur_constr_ID] = AT_SITE;
                        Movement::Move::remove(move_ID);
                    }
                }
                else if (statuses[cur_constr_ID] == AT_SITE) {
					printf("at site\n");
                    if (
                        cur_build_item.mineral_cost() <= Economy::get_free_minerals()
                        && cur_build_item.gas_cost() <= Economy::get_free_gas()
                    ) {
                        // TODO: final check that building here is OK
                        const BWAPI::UnitType &build_type = cur_build_item.unit_type();
                        const BWAPI::TilePosition &build_loc = build_locations[cur_constr_ID];
                        BWAPI::Unit builder = builders[cur_constr_ID];
                        builder->build(build_type, build_loc);
                        statuses[cur_constr_ID] = GIVEN_BUILD_CMD;
                        Basic::Units::set_cmd_delay(builder, BUILD_CMD_DELAY);
                    }
                }
                else if (statuses[cur_constr_ID] == GIVEN_BUILD_CMD) {
                    const BWAPI::Unit &builder = builders[cur_constr_ID];
                    const BWAPI::UnitType unit_type = builder->getType();
                    if (unit_type.isBuilding()) {
                        statuses[cur_constr_ID] = BUILDING;
                        Basic::Units::set_cmd_delay(builder, unit_type.buildTime() + FINISH_BUILD_FRAMES);
						//BuildOrder::next();
                    }
                    // TODO: deal with not advancing to 'BUILDING' status after some time
                }
                else if (statuses[cur_constr_ID] == BUILDING) {
                    const BWAPI::Unit &builder = builders[cur_constr_ID];
                    auto &unit_data = Basic::Units::data(builder);
                    if (unit_data.cmd_ready) {statuses[cur_constr_ID] = COMPLETED;}
                }
                else if (statuses[cur_constr_ID] == COMPLETED) {
                    // status is COMPLETED for one frame to allow for other sections to use that
                    // information
                    remove_build(build_ID);
                }
            }
            else {
                // TODO: ?
            }
        }

		void check_paths() {
			// TODO: (for now, just make sure a path is created)
			for (int i = 0; i < move_IDs.size(); ++i) {
				int move_ID = move_IDs[i];
				if (move_ID < 0) {
					printf("move ID < 0\n");
                    move_IDs[i] = Movement::Move::move(builders[i], build_locations[i]);
				}
			}
		}
    }

    void init() {
        int map_base_ct = Basic::Bases::all_bases().size();
        unused_workers.resize(map_base_ct);
    }

    void on_frame_update() {
		check_timer.on_frame_update();
		if (check_timer.is_stopped()) {
			check_paths();
			check_timer.start(0, CHECK_INTERVAL);
		}
        resolve_dead_builders();
        init_builds();
        advance_builds();
    }

    bool worker_reserved_for_building(const BWAPI::Unit unit) {
        for (auto &builder : builders) {
            if (unit == builder) {return true;}
        }
        return false;
    }




}