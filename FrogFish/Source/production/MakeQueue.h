#pragma once
#pragma message("Makequeue")

#include "../unitdata/FrogBase.h"
#include "../unitdata/UnitStorage.h"
#include "../unitdata/BaseStorage.h"
#include <BWAPI.h>
#include <vector>
#include <map>
#include <deque>

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
    void take_proportion_order(
        BaseStorage &base_storage,
        const std::vector<double> &proportions,
        const std::vector<bool> &high_priority,
        int order_period
    ) {
        queue.clear();
        queue_size = 0;
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

    void take_exact_order(const std::vector<UnitType> &unit_types) {
        queue.clear();
        for (auto &ut : unit_types) {
            queue.push_back(ut);
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
};

