#include "MakeUnits.h"
#include "Economy.h"
#include "BuildOrder.h"
#include "../basic/Bases.h"
#include "../utility/BWTimer.h"
#include <BWAPI.h>
#include <vector>

namespace Production::MakeUnits {

    namespace {
        // TODO: do some science, fix numbers!
        const int EXTRA_DELAY_FRAMES = 40;
        const int OVERLORD_SUPPLY_PROVIDED = 16;
        const int OVERLORD_MAKE_SEC = 48;

        void auto_insert_overlords() {
            int overlord_supply_incoming = 0;
            for (unsigned bo_index = BuildOrder::current_index(); bo_index < BuildOrder::size(); ++bo_index) {
                if (BuildOrder::get(bo_index).unit_type() == BWAPI::UnitTypes::Zerg_Overlord) {
                    overlord_supply_incoming += OVERLORD_SUPPLY_PROVIDED;
                }
                else {break;}
            }
            // TODO: fix
            int seconds_until_blocked = 1000; //Economy::seconds_until_supply_blocked();
            int supply_deficit = Broodwar->self()->supplyTotal() + overlord_supply_incoming 
                - Broodwar->self()->supplyUsed();
            if (supply_deficit < 0) {
                int supply_needed = supply_deficit * -1;
                int needed_overlords = (int)ceil((float)supply_needed / OVERLORD_SUPPLY_PROVIDED);
                for (int i = 0; i < needed_overlords; ++i) {
                    BuildOrder::insert_next(
                        BuildOrder::Item::MAKE,
                        BWAPI::UnitTypes::Zerg_Overlord,
                        BWAPI::TechTypes::None,
                        BWAPI::UpgradeTypes::None,
                        -1
                    );
                }
            }
            else if (seconds_until_blocked > -1 && seconds_until_blocked <= OVERLORD_MAKE_SEC) {
                BuildOrder::insert_next(
                    BuildOrder::Item::MAKE,
                    BWAPI::UnitTypes::Zerg_Overlord,
                    BWAPI::TechTypes::None,
                    BWAPI::UpgradeTypes::None,
                    -1
                );
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
                        printf("making %s\n", item.unit_type().c_str());
                        larva->morph(item.unit_type());
                        Basic::Units::set_utask(larva, Basic::Refs::MAKE);
                        Basic::Units::set_cmd_delay(larva, item.unit_type().buildTime() + EXTRA_DELAY_FRAMES);
                        BuildOrder::next();
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

    void on_frame_update() {
        //auto_insert_overlords();
        spend_down();
    }
}