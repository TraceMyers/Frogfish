#pragma once
#pragma message("including Makequeue")

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

// TODO: ProductionQueue

class MakeQueue {

private:

    std::deque<BWAPI::UnitType> queue;
    int queue_size;

public:

    const static int MKQ_UNIT_TYPE_CT = 8;

    const static BWAPI::UnitType types[8];

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
    );   
    void take_exact_order(const std::vector<UnitType> &unit_types);
    const std::deque<BWAPI::UnitType> &get_queue();
    void pop();
    BWAPI::UnitType front();
    void push_overlords_front(int count);
    bool order_filled();
    int size();
};

