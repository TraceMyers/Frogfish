#pragma once
#pragma message ("ConstructionManager")

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
    std::vector<FUnit> build_drones;
    std::vector<FUnit> under_construction;
    std::vector<BWAPI::TilePosition> build_tilepositions;
    bool building_init = false;
    bool building = false;
    ConstructionStorage construction_storage;

    void remove_dead_drones(UnitStorage &unit_storage);

public:

    void take_build_order(BuildOrder *_build_order);

    void build_structures(
        EconTracker &econ_tracker, 
        BaseStorage &base_storage,
        UnitStorage &unit_storage
    );
};