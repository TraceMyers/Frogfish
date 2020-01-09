#pragma once

#include <deque>
#include <BWAPI.h>

class BuildQueue {

private:

    std::deque<BWAPI::UnitType> queue;

public:

    const std::deque<BWAPI::UnitType> &get_queue() {
        return queue;
    }
};