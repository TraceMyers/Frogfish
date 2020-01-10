#pragma once

#include "MakeQueue.h"

class ProductionCoordinator {

private:

std::vector<double> make_proportions;
std::vector<bool> make_priorities;

public:

    ProductionCoordinator() :
        make_proportions(MakeQueue::MKQ_UNIT_TYPE_CT),
        make_priorities(MakeQueue::MKQ_UNIT_TYPE_CT)
    {
        make_proportions[MakeQueue::DRONE] = 1.0;
    }

    const std::vector<double> &get_make_proportions() {
        return make_proportions;
    }

    const std::vector<bool> &get_make_priorities() {
        return make_priorities;
    }
};