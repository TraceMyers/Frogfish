#include "utility/DebugDraw.h"
#include "utility/BWTimer.h"
#include "unitdata/UnitStorage.h"
#include "unitdata/BaseStorage.h"
#include "unitdata/TechStorage.h"
#include "unitdata/BaseOwnership.h"
#include "production/ProductionCoordinator.h"
#include "production/BuildPlacement.h"
#include "control/WorkerControl.h"
#include "unitdata/BWEMBaseArray.h"
#include <BWAPI.h>
#include <iostream>
#include <string>
#include <vector>

using namespace Filter;
// TODO:
//  - need to expand the features of pathfinding beyond simply providing paths.
//  - pathfinding needs to solve problems like: getting a unit from point a
//  to point b while avoiding potential and known danger. This would mean things like
//  risk-averse pathing, (moderately well-)known & seen danger avoidance, and
//  connecting with battle sim to decide if any path between a and b is too dangerous
//  for the unit to attempt without army intervention
//  - For now, pathing just needs to handle moving units from a to b with either
//  attacking or not attacking in mind

UnitStorage unit_storage;
BaseStorage base_storage;
TechStorage tech_storage;
ProductionCoordinator production_coordinator;
BWTimer timer;


void FrogFish::onStart() {
    Broodwar->sendText("Hello Sailor!");
    Broodwar->enableFlag(Flag::UserInput);
    Broodwar->setLocalSpeed(12);
    Broodwar->setCommandOptimizationLevel(2);
    onStart_alloc_debug_console();
    onStart_send_workers_to_mine();
    onStart_init_bwem_and_bweb();
    base_storage.init();
    BuildPlacement::init_base_ground_distances(base_storage);
    production_coordinator.init();
    production_coordinator.load_build_order("protoss", "12_hatch");
    timer.start(10,0);
}

void FrogFish::onFrame() {
	if (Broodwar->isReplay() || Broodwar->isPaused() || !Broodwar->self()) {return;}
    timer.on_frame_update();

    // 1. update basic data that everything else references
    unit_storage.update();
    BaseOwnership::update_base_data(base_storage, unit_storage);
    tech_storage.on_frame_update(base_storage);

    // 2. update production data
	production_coordinator.on_frame_update(base_storage, unit_storage);

    // PRE-LAST. draw
    // DebugDraw::draw_build_graphs();
    // DebugDraw::draw_units(unit_storage);
    // DebugDraw::draw_base_info(base_storage);

    // LAST. 
    // clear storage of changes to basic data that everybody else references
    // once others have had a chance to use that info
    unit_storage.clear_newly_assigned();
    base_storage.clear_newly_assigned();

    if (Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0) {return;}
    // RUN COMMANDS -----------------------------------------------------------------
    production_coordinator.produce(base_storage, unit_storage, tech_storage);
    WorkerControl::send_idle_workers_to_mine(base_storage);
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
    BWEB::Map::onUnitDiscover(unit);
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
        unit_storage.queue_remove(unit);
        the_map.OnGeyserNoticed(unit);
    }
}

void FrogFish::onUnitDestroy(BWAPI::Unit unit) {
    unit_storage.queue_remove(unit);
    // TODO: doesn't catch cases where destroyed out of vision!
    if (unit->getType().isMineralField()) {the_map.OnMineralDestroyed(unit);}
    else if (unit->getType().isSpecialBuilding()) {the_map.OnStaticBuildingDestroyed(unit);}
    BWEB::Map::onUnitDestroy(unit);
}

void FrogFish::onUnitMorph(BWAPI::Unit unit) {
    unit_storage.queue_store(unit);
    if (unit->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser) {
        unit_storage.queue_remove(unit);
        the_map.OnGeyserNoticed(unit);
    }
    BWEB::Map::onUnitMorph(unit);
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