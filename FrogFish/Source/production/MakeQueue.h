#pragma once

#include "../data/FrogBase.h"
#include "../data/UnitStorage.h"
#include "../data/BaseStorage.h"
#include <BWAPI.h>
#include <vector>
#include <map>
#include <deque>
// make units by proportions per UnitType
// try to fairly fulfill the proportions with limited larva per command frame
// don't just fill out one type in order before moving on to next,
// otherwise a type might constantly get passed up when new proportion orders
// are sent
// UnitType proportions -> static order
//      lowest proportion brought up to 1
//      all other proportions scaled up the same way and rounded
//      makes an order to be filled out
//      make one of each type in rounds unless the type's order is filled, then
//      skip over it
// drones are produced at bases where they are needed the most
//      assign all of the neediest base's larva to drone production
// all units can be required to be made at a specific base, which reserves
// larva for that unit type at that base (overrides drone priority)
// (decision to drone in bursts, save for mutas, etc. made elsewhere)
//      for example: save for mutas is as simple as setting the Mutalisk proportion to 1
//      before the unit can be built

//      if saving needs to happen, but only to a proportion of production,
//      larva can be reserved for a unittype until it becomes available,
//      either at specific bases or not 
// overlords are produced when:
    // unit production econ capacity, along with current larva + incoming larva over time
        // and current unit order shows that another order of the same kind
        // would hit a supply block
        // larva spawn rate: 1 per 340 frames
    // overlords are given high priority at the safest bases 

#ifndef LARVA_SPAWN_SEC
#define LARVA_SPAWN_SEC 14.166
#endif

class MakeQueue {

private:

    std::deque<BWAPI::UnitType> queue;
    int queue_size;

public:

    const static int MKQ_UNIT_TYPE_CT = 8;

    BWAPI::UnitType types[8] {
        BWAPI::UnitTypes::Zerg_Drone,
        BWAPI::UnitTypes::Zerg_Zergling,
        BWAPI::UnitTypes::Zerg_Hydralisk,
        BWAPI::UnitTypes::Zerg_Mutalisk,
        BWAPI::UnitTypes::Zerg_Scourge,
        BWAPI::UnitTypes::Zerg_Defiler,
        BWAPI::UnitTypes::Zerg_Ultralisk,
        BWAPI::UnitTypes::Zerg_Queen
    };

    enum TYPENAMES {
        DRONE,
        ZERGLING,
        HYDRALISK,
        MUTALISK,
        SCOURGE,
        DEFILER,
        ULTRALISK,
        QUEEN
    };

    // seems a bigger order period simply provides a higher resolution
    // if no high priority units are in the queue, and if no
    // unittypes that can't be made are in the queue
    // if either high priority or can't make exist in queue, 
    // the order period needs to be accurate
    // NOTE: unit proportions are essentially de-prioritizing if
    // put in in equal rounds; higher proportions only resolve later in the queue
    void take_order(
        BaseStorage &base_storage,
        const std::vector<double> &proportions,
        const std::vector<bool> &high_priority,
        int order_period
    ) {
        assert(proportions.size() == (unsigned int)MKQ_UNIT_TYPE_CT);
        const std::vector<FBase> &bases = base_storage.get_self_bases();
        register int larvae_ct = 0;
        register int resource_depot_ct = 0;
        for (auto base : bases) {
            larvae_ct += base->get_larva_ct();
            resource_depot_ct += base->get_resource_depot_ct();
        }
        int base_ct = bases.size();
        larvae_ct = (larvae_ct < base_ct ? base_ct : larvae_ct);
        int order_size = 
            larvae_ct + (int) round(resource_depot_ct * order_period / LARVA_SPAWN_SEC);
        
        std::vector<int> unit_type_make_counts(MKQ_UNIT_TYPE_CT);
        for (int i = 0; i < MKQ_UNIT_TYPE_CT ; ++i) {
            if (proportions[i] == 0) {
                unit_type_make_counts[i] = 0;
            }
            else {
                int n = (int) round(proportions[i] * order_size);
                if (n == 0) {
                    n = 1;
                }
                unit_type_make_counts[i] = n;
            }
        }

        bool still_filling_queue = true;
        while (still_filling_queue) {
            still_filling_queue = false;
            for (int i = 0; i < MKQ_UNIT_TYPE_CT; ++i) {
                const BWAPI::UnitType t = types[i];
                if (high_priority[i]) {
                    still_filling_queue = true;
                    while (unit_type_make_counts[i] > 0) {
                        queue.push_front(types[i]);
                        --unit_type_make_counts[i];
                    }
                }
                else if (unit_type_make_counts[i] > 0) {
                    still_filling_queue = true;
                    queue.push_back(types[i]);
                    --unit_type_make_counts[i];
                }
            }
        }
        queue_size = queue.size();
    }

    const std::deque<BWAPI::UnitType> &get_queue() {
        return queue;
    }

    void pop() {
        if (queue_size > 0) {
            queue.pop_front();
            --queue_size;
        }
    }

    BWAPI::UnitType front() {
        if (queue_size > 0) {
            return queue.front();
        }
        return BWAPI::UnitTypes::Unknown;

    }

    void push_overlords_front(int count) {
        for (int i = 0; i < count; i++) {
            queue.push_front(BWAPI::UnitTypes::Zerg_Overlord);
        }
    }

    bool order_filled() {
        return queue_size == 0;
    }

    int size() {
        return queue_size;
    }

    // overlord production assumes this order is going to be wanted forever
    // , projects larva production and mineral/gas spending capacity
    // to buffer


};

