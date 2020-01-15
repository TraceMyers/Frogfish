#include "FrogFish.h"
#include "utility/DebugDraw.h"
#include "utility/BWTimer.h"
#include "unitdata/UnitStorage.h"
#include "unitdata/BaseStorage.h"
#include "unitdata/BaseOwnership.h"
#include "production/ProductionCoordinator.h"
#include "control/WorkerControl.h"
#include <BWAPI.h>
#include <iostream>
#include <string>
#include <set>
#include <chrono>
#include <vector>
#include <fstream>

using namespace Filter;

UnitStorage unit_storage;
BaseStorage base_storage;
ProductionCoordinator production_coordinator;
BWTimer timer;

bool first_frame_over = false;
int node_i = 0;

void FrogFish::onStart() {
    Broodwar->sendText("Hello Sailor!");
    Broodwar->enableFlag(Flag::UserInput);
    Broodwar->setLocalSpeed(12);
    Broodwar->setCommandOptimizationLevel(2);
    onStart_alloc_debug_console();
    onStart_send_workers_to_mine();
    onStart_init_bwem();
    base_storage.init();
    timer.start(0, 5);
    production_coordinator.init();
    production_coordinator.load_build_order("terran", "9_pool");
}

void FrogFish::onFrame() {
	if (Broodwar->isReplay() || Broodwar->isPaused() || !Broodwar->self()) {return;}
    timer.on_frame_update();

    // update data
    unit_storage.update();
    BaseOwnership::update_base_data(base_storage, unit_storage);
    production_coordinator.on_frame_update(base_storage);


    // draw
    DebugDraw::draw_build_graphs();
    // DebugDraw::draw_units(unit_storage);
    // DebugDraw::draw_base_info(base_storage);

    // Delete units from newly_stored and newly_removed lists
    // after other functions have gotten a chance to see that they're
    // being removed
    unit_storage.clear_newly_assigned();
    base_storage.clear_newly_assigned();

    if (Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0) {return;}
    // RUN COMMANDS -----------------------------------------------------------------
    production_coordinator.produce(base_storage);
    send_idle_workers_to_mine(base_storage);
}

void FrogFish::onSendText(std::string text) {
    Broodwar->sendText("%s", text.c_str());
}

void FrogFish::onReceiveText(BWAPI::Player player, std::string text) {
    Broodwar << player->getName() << " said \"" << text << "\"" << std::endl;
}

void FrogFish::onPlayerLeft(BWAPI::Player player) {
    Broodwar->sendText("Goodbye %s!", player->getName().c_str());
}

void FrogFish::onNukeDetect(BWAPI::Position target) {
    if (target) {
        Broodwar << "Nuclear Launch Detected at " << target << std::endl;
    }
    else {
        Broodwar->sendText("Where's the nuke?");
    }
}

void FrogFish::onUnitDiscover(BWAPI::Unit unit) {
    unit_storage.queue_store(unit);
    if (unit->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser) {
        the_map.OnGeyserNoticed(unit);
    }
}

void FrogFish::onUnitEvade(BWAPI::Unit unit) {
    // pass
}

void FrogFish::onUnitShow(BWAPI::Unit unit) {
    // pass
}

void FrogFish::onUnitHide(BWAPI::Unit unit) {
    // pass
}

void FrogFish::onUnitCreate(BWAPI::Unit unit) {
    unit_storage.queue_store(unit);
    if (unit->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser) {
        the_map.OnGeyserNoticed(unit);
    }
}

void FrogFish::onUnitDestroy(BWAPI::Unit unit) {
    unit_storage.queue_remove(unit);
    // TODO: doesn't catch cases where destroyed out of vision!
    if (unit->getType().isMineralField()) {the_map.OnMineralDestroyed(unit);}
    else if (unit->getType().isSpecialBuilding()) {the_map.OnStaticBuildingDestroyed(unit);}
}

void FrogFish::onUnitMorph(BWAPI::Unit unit) {
    unit_storage.queue_store(unit);
    if (unit->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser) {
        the_map.OnGeyserNoticed(unit);
    }
}

void FrogFish::onUnitRenegade(BWAPI::Unit unit) {
    // pass
}

void FrogFish::onSaveGame(std::string gameName) {
    // pass
}

void FrogFish::onUnitComplete(Unit unit) {
    // broadcast for structure usage
}

void FrogFish::onEnd(bool isWinner) {
    FreeConsole();
    unit_storage.free_data();
    base_storage.free_data();
}