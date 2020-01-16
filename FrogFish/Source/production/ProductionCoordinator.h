#pragma once
#pragma message("ProductionCoordinator")

#include "BuildOrder.h"
#include "UnitMaker.h"
#include "EconTracker.h"
#include "UnitMaker.h"
#include "ConstructionManager.h"
#include "../unitdata/BaseStorage.h"
#include "../unitdata/UnitStorage.h"

// has the ability to switch between build orders and real-time plans
// for instance, though the bot always starts with a build order, it may
// need to interrupt the build order to react to an enemy attack. It could
// then potentially continue the build order (or another BO) after that.
class ProductionCoordinator {

private:

    BuildOrder build_order;
    EconTracker econ_tracker;
    UnitMaker unit_maker;
    ConstructionManager construction_manager;

    std::vector<double> make_proportions;
    std::vector<bool> make_priorities;

public:

    void init();

    void load_build_order(const char *race, const char *name);

    void on_frame_update(BaseStorage &base_storage);

    void produce(BaseStorage &base_storage, UnitStorage &unit_storage);

    const std::vector<double> &get_make_proportions();

    const std::vector<bool> &get_make_priorities();
};