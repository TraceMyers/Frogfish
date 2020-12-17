#include "MakeUnits.h"
#include "Economy.h"
#include "BuildOrder.h"
#include "../basic/Bases.h"
#include "../utility/BWTimer.h"
#include "../test/TestMessage.h"
#include <BWAPI.h>
#include <vector>

namespace Production::MakeUnits {

    namespace {
        // TODO: do some science, fix numbers!
        const int EXTRA_DELAY_FRAMES = 10;
        const int OVERLORD_MAKE_SECONDS = 20;
        const int OVERLORD_COST = 100;
        BWTimer overlord_push_delay;

        // inserts one overlord if needed - only over the next interval where
        // there is no block on making overlords
        // TODO: inserting overlords can significantly delay construction; units on way to build
        // site need to be potentially pulled back
        void auto_insert_overlords(const std::vector<std::pair<int, int>> &econ_sim_data) {
            int supply_block_seconds = Economy::seconds_until_supply_blocked();
            if (0 <= supply_block_seconds && supply_block_seconds < 100) { // < 0 means no block over sim duration
                int supply_block_ID = Economy::build_order_ID_at_supply_block();
                bool overlord_make_block = BuildOrder::overlord_make_block();
                float minerals_per_sec = Economy::get_minerals_per_sec();
                int cur_ID = BuildOrder::current_index();
                int last_ID_can_insert = (overlord_make_block ? -1 : cur_ID);
                int supply_block_sim_index = econ_sim_data.size() - 1;
                int target_time = supply_block_seconds - OVERLORD_MAKE_SECONDS;
                bool successful_insert = false;

                for (unsigned BO_ID = cur_ID, sim_index = 0; BO_ID < supply_block_ID; ++BO_ID, ++sim_index) {
                    const BuildOrder::Item &item = BuildOrder::get(BO_ID);
                    if (item.action() == BuildOrder::Item::OVERLORD_MAKE_BLOCK_ON) {
                        overlord_make_block = true;
                    }
                    else if (item.action() == BuildOrder::Item::OVERLORD_MAKE_BLOCK_OFF) {
                        overlord_make_block = false;
                    }
                    if (!overlord_make_block) {
                        last_ID_can_insert = BO_ID;
                    }
                   
                    int time_until_make = econ_sim_data[sim_index].second;
                    if (time_until_make >= target_time) {
                        if (last_ID_can_insert > 0) {
                            bool moved_late_overlord_back = false;
                            for (int i = last_ID_can_insert; i < BuildOrder::size(); ++i) {
                                auto &item = BuildOrder::get(i);
                                if (item.unit_type() == BWAPI::UnitTypes::Zerg_Overlord) {
                                    BuildOrder::move(i, last_ID_can_insert);
                                    moved_late_overlord_back = true;
                                    break;
                                }
                            }
                            if (!moved_late_overlord_back) {
                                BuildOrder::insert(
                                    BuildOrder::Item::MAKE,
                                    BWAPI::UnitTypes::Zerg_Overlord,
                                    BWAPI::TechTypes::None,
                                    BWAPI::UpgradeTypes::None,
                                    -1,
                                    last_ID_can_insert
                                );
                            }
                            int overlord_delay_seconds = (int)((1/minerals_per_sec) * OVERLORD_COST);
                            Economy::add_delay_to_build_order_sim(
                                last_ID_can_insert + 1, 
                                overlord_delay_seconds, 
                                true
                            );
                            Economy::sim_set_just_added_overlord_flag_true();
                            successful_insert = true;
                            break;
                        }
                    }
                }

                if (!successful_insert) {
                    DBGMSG("auto_insert_overlords(): unsolvable block!\n");
                }
            }
        }

        // TODO: get larva from map analysis, not by selecting first available
        void spend_down() {
            bool still_spending = true;
            for (auto &base : Basic::Bases::self_bases()) {
                for (auto &larva : Basic::Bases::larva(base)) {
                    if (BuildOrder::finished()) {
                        still_spending = false;
                        break;
                    }
                    auto &item = BuildOrder::current_item();
                    if (
                        item.action() == BuildOrder::Item::MAKE
                        && Basic::Tech::self_can_make(item.unit_type())
                        && item.mineral_cost() <= Economy::get_free_minerals()
                        && item.gas_cost() <= Economy::get_free_gas()
                        && Broodwar->self()->supplyUsed() + item.supply_cost() 
                            <= Broodwar->self()->supplyTotal()
                    ) {
                        larva->morph(item.unit_type());
                        Basic::Units::set_utask(larva, Basic::Refs::MAKE);
                        Basic::Units::set_cmd_delay(larva, item.unit_type().buildTime() + EXTRA_DELAY_FRAMES);
                        BuildOrder::next();
                        if (item.unit_type() == BWAPI::UnitTypes::Zerg_Overlord) {
                            printf("here\n");
                            overlord_push_delay.start(1, 0);
                        }
                    }
                    else {
                        still_spending = false;
                        break;
                    }
                }
                if (!still_spending) {break;}
            }
        }
    }

    void init() {
        // TODO: do away with this hacky bullshit
        overlord_push_delay.start(10, 0);
    }

    void on_frame_update() {
        spend_down();
        const std::vector<std::pair<int, int>> &econ_sim_data = Economy::get_sim_data();
        overlord_push_delay.on_frame_update();
        if (overlord_push_delay.is_stopped()) {
            auto_insert_overlords(econ_sim_data);
        }
    }
}