#include "UnitMaker.h"
#include "MakeQueue.h"
#include "MorphQueue.h"
#include "EconTracker.h"
#include "BuildOrder.h"
#include "../unitdata/BaseStorage.h"
#include "../utility/BWTimer.h"
#include <BWAPI.h>
#include <vector>
#include <deque>

#define CHANGE_MAGIC_NUMBER 40
#define EXTRA_DELAY_FRAMES 50
#define OVERLORD_TIMER_CT 20
#define OVERLORD_SUPPLY_PROVIDED 16
#define OVERLORD_MAKE_SEC 48.0
#ifndef LARVA_SPAWN_SEC 
#define LARVA_SPAWN_SEC 14.166
#endif

void UnitMaker::auto_push_overlord(EconTracker &econ_tracker) {
    BWAPI::Player self = Broodwar->self();
    int making_overlords = 0;
    for (auto &timer : overlord_making_timers) {
        if (!timer.is_stopped()) {
            ++making_overlords;
        }
    }
    int 
        used_supply = self->supplyUsed(),
        supply_total = self->supplyTotal() + making_overlords * OVERLORD_SUPPLY_PROVIDED; 
    double sps = econ_tracker.get_supply_per_sec();
    for(auto &item : make_queue.get_queue()) {
        if (item == BWAPI::UnitTypes::Zerg_Overlord) {
            supply_total += OVERLORD_SUPPLY_PROVIDED;
        }
        else {
            break;
        }
    }
    double supply_in_build_time = (double) used_supply + sps * OVERLORD_MAKE_SEC - 3;
    if (supply_in_build_time > supply_total) {
        printf("%lf, %d\n", supply_in_build_time, supply_total);
        printf("sps: %.3lf\n", sps);
        make_queue.push_overlords_front(1);
        for (auto &timer : overlord_making_timers) {
            if (timer.is_stopped()) {
                timer.start((int)OVERLORD_MAKE_SEC, 0);
                break;
            }
        }
    }
}

// for now, just make anywhere
// probably very temp
void UnitMaker::spend_down(BaseStorage &base_storage, EconTracker &econ_tracker) {
    bool still_spending = true;
    for (auto &base : base_storage.get_self_bases()) {
        for (auto &larva : base->get_larva()) {
            if (make_queue.order_filled()) {
                still_spending = false;
                break;
            }
            BWAPI::UnitType next_unit = make_queue.front();
            if (next_unit.mineralPrice() <= econ_tracker.get_free_minerals()
                && next_unit.gasPrice() <= econ_tracker.get_free_gas()
                && Broodwar->self()->supplyUsed() + next_unit.supplyRequired()
                <= Broodwar->self()->supplyTotal()
            ) {
                larva->set_cmd_delay(next_unit.buildTime() + EXTRA_DELAY_FRAMES);
                larva->bwapi_u()->morph(next_unit);
                make_queue.pop();
            }
            else {
                still_spending = false;
                break;
            }
        }
        if (!still_spending) {
            break;
        }
    }
}


UnitMaker::UnitMaker() :
    overlord_making_timers(OVERLORD_TIMER_CT),
    mode(BUILD_ORDER)
{}

void UnitMaker::set_mode(MODE _mode) {
    mode = _mode;
}

void UnitMaker::on_frame_update() {
    for (auto &timer : overlord_making_timers) {
        timer.on_frame_update();
    } 
}

void UnitMaker::take_proportional_order(
    BaseStorage &base_storage,
    const std::vector<double> &make_proportions,
    const std::vector<bool> &make_priorities
) {
    make_queue.take_proportion_order(
        base_storage, 
        make_proportions, 
        make_priorities, 
        CHANGE_MAGIC_NUMBER
    );
}

void UnitMaker::take_build_order(BuildOrder *_build_order) {
    build_order = _build_order;
}

void UnitMaker::make_units(EconTracker &econ_tracker, BaseStorage &base_storage) {
    if (mode != PAUSED) {
        if (mode == PROPORTIONAL) {
            auto_push_overlord(econ_tracker);
        }
        else if (make_queue.order_filled()) {
            BuildItem &build_item = build_order->peek_next();
            int supply_used = Broodwar->self()->supplyUsed();
            if (build_item.build_type == BuildItem::MAKE_UNIT) {
                if (supply_used == build_item.supply_target) {
                    build_order_fill_queue();
                }
            }
            else if (!finished_init_droning
                && supply_used < build_order->get(0).supply_target
            ) {
                int make_drone_ct = (build_item.supply_target - supply_used) / 2;
                std::vector<UnitType> drones;
                for (int i = 0; i < make_drone_ct; ++i) {
                    drones.push_back(BWAPI::UnitTypes::Zerg_Drone);
                }
                make_queue.take_exact_order(drones);
                finished_init_droning = true;
            }
        }
        if (!make_queue.order_filled()) {
            spend_down(base_storage, econ_tracker);
        }
    }
}

void UnitMaker::build_order_fill_queue() {
    std::vector<BWAPI::UnitType> unit_types;
    while (!build_order->finished()) {
        BuildItem &build_item = build_order->peek_next();
        if (build_item.build_type == BuildItem::MAKE_UNIT) {
            unit_types.push_back(build_item.make_type);
            build_order->next();
        }
        else {
            break;
        }
    }    
    make_queue.take_exact_order(unit_types);
}

// unit maker and building maker both know what op codes they use, and
// they wait until the build order counter reaches the next item they can handle
