#include "ProductionCoordinator.h"
#include "BuildPlacement.h"
#include "EconTracker.h"
#include "ConstructionManager.h"
#include "MakeQueue.h"
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
    construction_manager.on_frame_update(base_storage, unit_storage, econ_tracker);
    test_timer.on_frame_update();
}


void ProductionCoordinator::produce(
    BaseStorage &base_storage, 
    UnitStorage &unit_storage,
    TechStorage &tech_storage
) {
    MakeQueue &make_queue = unit_maker.get_make_queue();
    econ_timing_estimates = econ_tracker.build_order_sim(
        unit_storage, 
        &build_order,
        make_queue
    );
    construction_manager.init_builds(
        base_storage, 
        &build_order, 
        econ_tracker, 
        econ_timing_estimates
    );
    unit_maker.make_units(econ_tracker, base_storage, tech_storage);
}

const std::vector<double> &ProductionCoordinator::get_make_proportions() {
    return make_proportions;
}

const std::vector<bool> &ProductionCoordinator::get_make_priorities() {
    return make_priorities;
}