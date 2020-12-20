#include "Construction.h"
#include "BuildOrder.h"
#include "Economy.h"
#include "BuildGraph.h"
#include "../movement/Move.h"
#include "../basic/Bases.h"
#include "../basic/Units.h"
#include "../strategy/ConstructionPlanning.h"
#include "../test/TestMessage.h"
#include <BWEB/BWEB.h>
#include <BWAPI.h>
#include <assert.h>

using namespace Strategy;

namespace Production::Construction {
    namespace {

        std::vector<int>                        construction_plan_IDs;
        std::vector<int>                        move_IDs;
        std::vector<int>                        buildgraph_reservation_IDs;
        std::vector<BWAPI::Unit>                canceled_build_units;
        const int                               FINISH_BUILD_FRAMES = 30;
        const int                               BUILD_CMD_DELAY = 2;
        const int                               NOT_FOUND = -1;
        BWTimer                                 shit_timer;

        void init_builds() {
            for (
                unsigned build_index = BuildOrder::current_index(); 
                build_index < BuildOrder::size(); 
                ++build_index
            ) {
                const BuildOrder::Item &build_item = BuildOrder::get(build_index);
                if (
                    build_item.action() == BuildOrder::Item::BUILD 
                    && ConstructionPlanning::find_plan(build_item) < 0
                ) {
                    int plan_ID = ConstructionPlanning::make_construction_plan(build_item);
                    if (plan_ID < 0) {
                        DBGMSG(
                            "Construction::init_builds(): build order item ID %d Error %d\n",
                            build_item.ID(),
                            plan_ID
                        );
                        continue;
                    }
                    const ConstructionPlanning::ConstructionPlan &plan = ConstructionPlanning::get_plan(plan_ID);
                    const BWAPI::TilePosition& tp = plan.get_tilepos();
                    const BWAPI::Unit& builder = plan.get_builder();
                    int move_ID = Movement::Move::move(builder, tp, false, true);
                    if (move_ID < 0) {
                        // TODO: pathing error! need to deal with this somehow
                        DBGMSG("Construction::init_builds(): pathing error to (%d, %d)\n", tp.x, tp.y);
                        ConstructionPlanning::destroy_plan(plan_ID);
                        continue;
                    }

                    // reserve build space
                    const BWAPI::UnitType &type = build_item.unit_type();
                    int buildgraph_res_ID = BuildGraph::make_reservation(
                        plan.get_base(),
                        tp,
                        type.tileWidth(),
                        type.tileHeight()
                    );
                    if (buildgraph_res_ID < 0) {
                        DBGMSG("Construction::init_builds(): buildgraph res error to (%d, %d)\n", tp.x, tp.y);
                        ConstructionPlanning::destroy_plan(plan_ID);
                        Movement::Move::remove(move_ID);
                        continue;
                    }
                    // reserve builder drone
                    auto& unit_data = Basic::Units::data(builder);
                    Basic::Units::set_build_status(builder, Basic::Refs::BUILD_STATUS::RESERVED);

                    move_IDs.push_back(move_ID);
                    construction_plan_IDs.push_back(plan_ID);
                    buildgraph_reservation_IDs.push_back(buildgraph_res_ID);
                } 
            }
        }

        // TODO: account for bad or messed up time predictions
        // TODO: account for deaths
        void advance_builds() {
            // if there is a 'cancel' build order item, do the cancel!
            /*
            auto &cur_build_item = BuildOrder::current_item();
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
            */
            auto construction_plan_IDs_it = construction_plan_IDs.begin();
            auto move_IDs_it = move_IDs.begin();
            auto buildgraph_reservation_IDs_it = buildgraph_reservation_IDs.begin();
            DBGMSG("Construction::advance_builds(): plans size: %d\n", construction_plan_IDs.size());
            while (construction_plan_IDs_it < construction_plan_IDs.end()) {
                int plan_ID = *construction_plan_IDs_it;
                auto& plan = ConstructionPlanning::get_plan(plan_ID);
                const BWAPI::Unit& builder = plan.get_builder();
                const Basic::Units::UnitData builder_data = Basic::Units::data(builder);
                auto& build_status = builder_data.build_status;
                if (build_status == BUILD_STATUS::RESERVED) {
                    auto& build_item = plan.get_item();
                    int build_time = build_item.seconds_until_make();
                    if (build_time != NOT_FOUND) {
                        int move_ID = *move_IDs_it;
                        int travel_time = Movement::Move::remaining_frames(move_ID);
                        int start_build_frames = build_time * 24;
                        if (start_build_frames <= travel_time) {
                            Movement::Move::start(move_ID);
                            Basic::Units::set_utask(builder, UTASK::BUILD);
                            Basic::Units::set_build_status(builder, BUILD_STATUS::MOVING);
                            DBGMSG("Construction::advance_builds(): moving\n");
                        }
                    }
                }
                else if (build_status == BUILD_STATUS::MOVING) {
                    int move_ID = *move_IDs_it;
                    // TODO: account for potential move errors
                    if (Movement::Move::get_status(move_ID) == Movement::Move::DESTINATION) {
                        Basic::Units::set_build_status(builder, BUILD_STATUS::AT_SITE);
                        // Could result in errors where bad ID is used, since the 
                        // bad move ID is kept for vector concurrency; might move this to the end
                        Movement::Move::remove(move_ID);
                        DBGMSG("Construction::advance_builds(): at site\n");
                    }
                }
                else if (build_status == BUILD_STATUS::AT_SITE) {
                    auto& build_item = plan.get_item();
                    if (
                        BuildOrder::current_item() == build_item
                        && build_item.mineral_cost() <= Economy::get_free_minerals()
                        && build_item.gas_cost() <= Economy::get_free_gas()
                    ) {
                        const BWAPI::UnitType &build_type = build_item.unit_type();
                        const BWAPI::TilePosition &build_loc = plan.get_tilepos();
                        builder->build(build_type, build_loc);
                        Basic::Units::set_build_status(builder, BUILD_STATUS::GIVEN_BUILD_CMD);
                        Basic::Units::set_cmd_delay(builder, BUILD_CMD_DELAY);
                        DBGMSG("Construction::advance_builds(): given build cmd\n");
                    }
                }
                else if(build_status == BUILD_STATUS::GIVEN_BUILD_CMD) {
                    DBGMSG("**1\n");
                    const BWAPI::UnitType unit_type = builder->getType();
                    auto &unit_data = Basic::Units::data(builder);
                    if (unit_type.isBuilding()) {
                        DBGMSG("**2\n");
                        // TODO: remove buildgraph reservation
                        Basic::Units::set_build_status(builder, BUILD_STATUS::BUILDING);
                        Basic::Units::set_cmd_delay(builder, unit_type.buildTime() + FINISH_BUILD_FRAMES);
                        DBGMSG("Construction::advance_builds(): building\n");
                        BuildOrder::next();
                    }
                    else if (unit_data.cmd_ready) {
                        DBGMSG("**3\n");
                        auto& build_item = plan.get_item();
                        const BWAPI::UnitType &build_type = build_item.unit_type();
                        const BWAPI::TilePosition &build_loc = plan.get_tilepos();
                        builder->build(build_type, build_loc);
                        Basic::Units::set_cmd_delay(builder, BUILD_CMD_DELAY);
                    }
                    // TODO: deal with not advancing to 'BUILDING' status after some time
                }
                else if (build_status == BUILD_STATUS::BUILDING) {
                    auto &unit_data = Basic::Units::data(builder);
                    if (unit_data.cmd_ready) {
                        Basic::Units::set_build_status(builder, BUILD_STATUS::COMPLETED);
                        DBGMSG("Construction::advance_builds(): finished building\n");
                    }
                }
                else if (build_status == BUILD_STATUS::COMPLETED) {
                    // TODO: builds just compeleted registry
                    ConstructionPlanning::destroy_plan(plan_ID);
                    buildgraph_reservation_IDs_it = buildgraph_reservation_IDs.erase(buildgraph_reservation_IDs_it);
                    construction_plan_IDs_it = construction_plan_IDs.erase(construction_plan_IDs_it);
                    move_IDs_it = move_IDs.erase(move_IDs_it);
                    if (construction_plan_IDs_it == construction_plan_IDs.end()) {
                        break;
                    }
                }
                else {
                    DBGMSG("Construction::advance_builds(): bad build_status: %d\n", build_status);
                }
                ++buildgraph_reservation_IDs_it;
                ++construction_plan_IDs_it;
                ++move_IDs_it;
            }
        }
    }

    // TODO: figure out a way to not have to wait for a second
    void init() {
        shit_timer.start(1, 0);
    }

    void on_frame_update() {
        shit_timer.on_frame_update();
        if (shit_timer.is_stopped() && !BuildOrder::finished()) {
            init_builds();
            advance_builds();
        }
        // TODO: babysit canceled build units until they've definitely stopped building
    }

    void cancel_build(const Production::BuildOrder::Item &item) {
        int plan_ID = ConstructionPlanning::find_plan(item);
        int move_ID;
        int buildgraph_ID;
        if (plan_ID >= 0) {
            auto construction_plan_IDs_it = construction_plan_IDs.begin();
            auto move_IDs_it = move_IDs.begin();
            auto buildgraph_reservation_IDs_it = buildgraph_reservation_IDs.begin();
            bool removed = false;
            while (construction_plan_IDs_it < construction_plan_IDs.end()) {
                if (*construction_plan_IDs_it == plan_ID) {
                    move_ID = *move_IDs_it;
                    buildgraph_ID = *buildgraph_reservation_IDs_it;
                    buildgraph_reservation_IDs.erase(buildgraph_reservation_IDs_it);
                    construction_plan_IDs.erase(construction_plan_IDs_it);
                    move_IDs.erase(move_IDs_it);
                    removed = true;
                    break;
                }
                ++buildgraph_reservation_IDs_it;
                ++construction_plan_IDs_it;
                ++move_IDs_it;
            }
            if (removed) {
                auto& plan = ConstructionPlanning::get_plan(plan_ID);
                auto& builder = plan.get_builder();
                auto& builder_data = Basic::Units::data(builder);
                if (builder_data.build_status == BUILD_STATUS::BUILDING) {
                    // important for extractors. can't wait until it's done automatically next frame
                    // or get read access violation
                    builder->cancelConstruction();
                    Basic::Bases::remove_struct_from_owner_base(builder);
                }
                else if (builder_data.build_status == BUILD_STATUS::GIVEN_BUILD_CMD) {
                    builder->stop();
                }
                Basic::Units::set_utask(builder, Basic::Refs::IDLE);
                Basic::Units::set_build_status(builder, Basic::Refs::BUILD_STATUS::NONE);
                canceled_build_units.push_back(builder);

                Movement::Move::remove(move_ID);
                ConstructionPlanning::destroy_plan(plan_ID);
                // TODO: unreserve spot in buildgraph
            }
            else {
                DBGMSG("Construction::cancel_build() unsuccessful removal\n");
            }
        }
        else {
            DBGMSG("Construction::cancel_build() can't find item.\n");
        }
    }
}
