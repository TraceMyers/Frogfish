#pragma once

#include <deque>
#include <BWAPI.h>

class MorphQueue {

private:

    std::deque<BWAPI::UnitType> queue;

public:

    const std::deque<BWAPI::UnitType> &get_queue() {
        return queue;
    }
};