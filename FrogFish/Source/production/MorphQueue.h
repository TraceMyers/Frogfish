#pragma once
#pragma message("including MophQueue")

#include "../unitdata/FrogUnit.h"
#include <BWAPI.h>
#include <deque>

// workers similarly to makequeue, except is constantly looking for targets to morph
// for each morph item in the queue for which it doesn't have a target, it sends a 
// signal out (like, in a vec of UnitTypes)
class MorphQueue {

private:

    std::deque<BWAPI::UnitType> queue;
    std::vector<FUnit> morph_targets;
    int queue_size;

public:

    const static int MRPHQ_UNIT_TYPE_CT = 6;

    const static BWAPI::UnitType types[6];

    enum TYPENAMES {
        LURKER,
        DEVOURER,
        GUARDIAN,
        LAIR,
        HIVE,
        GREATER_SPIRE
    };

    void take_exact_order(const std::vector<BWAPI::UnitType> &unit_types) {
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