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

    const int IN_BASE_TRAVEL_FRAMES = 70;

    ConstructionStorage construction_storage;

public:

    void on_frame_update(
        BaseStorage &base_storage,
        UnitStorage &unit_storage,
        EconTracker &econ_tracker
    );

    void init_builds(
        BaseStorage &base_storage,
        BuildOrder *build_order,
        EconTracker &econ_tracker,
        std::vector<std::vector<int>> &econ_time_est
    );
};