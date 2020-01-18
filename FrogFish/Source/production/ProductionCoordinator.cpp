#include "ProductionCoordinator.h"
#include "BuildPlacement.h"
#include "EconTracker.h"
#include "BuildOrder.h"
#include "ConstructionManager.h"
#include "../unitdata/BaseStorage.h"
#include "../unitdata/UnitStorage.h"

ProductionCoordinator::ProductionCoordinator() :
    make_proportions(),
    make_priorities(),
    econ_timing_estimates(),
    econ_tracker(),
    unit_maker(),
    build_order(),
    construction_manager()
{}

void ProductionCoordinator::init() {
    econ_tracker.init();
    test_timer.start(3, 0);
}

void ProductionCoordinator::load_build_order(const char *race, const char *build_name) {
    build_order.load(race, build_name);
    build_order.print();
    unit_maker.take_build_order(&build_order);
}

void ProductionCoordinator::on_frame_update(BaseStorage &base_storage, UnitStorage &unit_storage) {
    econ_tracker.on_frame_update(base_storage, unit_storage);
    BuildPlacement::on_frame_update(base_storage);
    unit_maker.on_frame_update();
    construction_manager.on_frame_update(base_storage, unit_storage);
    econ_timing_estimates = econ_tracker.build_order_sim(unit_storage, &build_order);
    test_timer.on_frame_update();
}

void ProductionCoordinator::produce(BaseStorage &base_storage, UnitStorage &unit_storage) {
    unit_maker.make_units(econ_tracker, base_storage);
    if (test_timer.is_stopped() && !init_test) {
        construction_manager.take_build_order(base_storage, &build_order, econ_timing_estimates);
        init_test = true;
    }
}

const std::vector<double> &ProductionCoordinator::get_make_proportions() {
    return make_proportions;
}

const std::vector<bool> &ProductionCoordinator::get_make_priorities() {
    return make_priorities;
}