#pragma once
#pragma message("including ConstructionManager")

#include "BuildOrder.h"
#include "EconTracker.h"
#include "../unitdata/FrogUnit.h"
#include "../unitdata/BaseStorage.h"
#include "../unitdata/UnitStorage.h"
#include "ConstructionStorage.h"
#include <BWAPI.h>

class ConstructionManager {

private:

    BuildOrder *build_order;
    ConstructionStorage construction_storage;

    void remove_dead_drones(UnitStorage &unit_storage);

public:

    void take_build_order(
        BaseStorage &base_storage,
        BuildOrder *_build_order,
        std::vector<std::vector<int>> &econ_timing_estimates
    );

    void on_frame_update(
        BaseStorage &base_storage,
        UnitStorage &unit_storage

    );

    void init_construction_default(
        BaseStorage &base_storage,
        std::vector<std::vector<int>> &econ_timing_estimates,
        BuildItem &item,
        int item_i
    );
};