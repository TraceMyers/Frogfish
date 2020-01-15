#include "ProductionCoordinator.h"
#include "BuildPlacement.h"
#include "EconTracker.h"
#include "BuildOrder.h"
#include "ConstructionManager.h"
#include "../unitdata/BaseStorage.h"

void ProductionCoordinator::init() {
    econ_tracker.init();
}

void ProductionCoordinator::load_build_order(const char *race, const char *build_name) {
    build_order.load(race, build_name);
    build_order.print();
    unit_maker.take_build_order(&build_order);
    construction_manager.take_build_order(&build_order);
}

void ProductionCoordinator::on_frame_update(BaseStorage &base_storage) {
    BuildPlacement::on_frame_update(base_storage);
    econ_tracker.on_frame_update();
    unit_maker.on_frame_update();
}

void ProductionCoordinator::produce(BaseStorage &base_storage) {
    unit_maker.make_units(econ_tracker, base_storage);
    construction_manager.build_structures(econ_tracker, base_storage);
}

const std::vector<double> &ProductionCoordinator::get_make_proportions() {
    return make_proportions;
}

const std::vector<bool> &ProductionCoordinator::get_make_priorities() {
    return make_priorities;
}