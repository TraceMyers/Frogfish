#include "FrogFish.h"
#include "utility/DebugDraw.h"
#include "utility/BWTimer.h"
#include "unitdata/UnitStorage.h"
#include "unitdata/BaseStorage.h"
#include "unitdata/EnemyBase.h"
#include "unitdata/BaseOwnership.h"
#include "production/EconTracker.h"
#include "production/UnitMaker.h"
#include "production/ProductionCoordinator.h"
#include "production/BuildGraph.h"
#include "production/BuildPlacement.h"
#include "control/WorkerControl.h"
#include <BWAPI.h>
#include <iostream>
#include <string>
#include <set>
#include <chrono>
#include <vector>

using namespace BWAPI;
using namespace Filter;

UnitStorage unit_storage;
BaseStorage base_storage;
EconTracker econ_tracker;
ProductionCoordinator production_coordinator;
UnitMaker unit_maker;
BWTimer<void *> timer;

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
    econ_tracker.init();
    timer.start(0, 5);
}

void FrogFish::onFrame() {
	if (Broodwar->isReplay() || Broodwar->isPaused() || !Broodwar->self()) {return;}
    timer.on_frame_update();

    // update data
    unit_maker.on_frame_update();
    econ_tracker.on_frame_update();    
    unit_storage.update();
    update_base_data(base_storage, unit_storage);
    unit_storage.clear_newly_assigned();
    BuildPlacement::on_frame_update(base_storage);
    base_storage.clear_newly_assigned();

    // draw
    
    DebugDraw::draw_units(unit_storage);
    DebugDraw::draw_base_info(base_storage);
    DebugDraw::draw_build_graphs(base_storage);

    if (Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0) {return;}
    // RUN COMMANDS -----------------------------------------------------------------

    send_idle_workers_to_mine(base_storage);

    // try contiunous drone production with current mechanisms:
    unit_maker.make_units(econ_tracker, base_storage, production_coordinator);
    
}

void FrogFish::onSendText(std::string text) {
    Broodwar->sendText("%s", text.c_str());
}

void FrogFish::onReceiveText(Player player, std::string text) {
    Broodwar << player->getName() << " said \"" << text << "\"" << std::endl;
}

void FrogFish::onPlayerLeft(Player player) {
    Broodwar->sendText("Goodbye %s!", player->getName().c_str());
}

void FrogFish::onNukeDetect(Position target) {
    if (target) {
        Broodwar << "Nuclear Launch Detected at " << target << std::endl;
    }
    else {
        Broodwar->sendText("Where's the nuke?");
    }
}

void FrogFish::onUnitDiscover(Unit unit) {
    unit_storage.queue_store(unit);
    if (unit->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser) {
        the_map.OnGeyserNoticed(unit);
    }
}

void FrogFish::onUnitEvade(Unit unit) {
    // pass
}

void FrogFish::onUnitShow(Unit unit) {
    // pass
}

void FrogFish::onUnitHide(Unit unit) {
    // pass
}

void FrogFish::onUnitCreate(Unit unit) {
    unit_storage.queue_store(unit);
    if (unit->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser) {
        the_map.OnGeyserNoticed(unit);
    }
}

void FrogFish::onUnitDestroy(Unit unit) {
    unit_storage.queue_remove(unit);
    // TODO: doesn't catch cases where destroyed out of vision!
    if (unit->getType().isMineralField()) {the_map.OnMineralDestroyed(unit);}
    else if (unit->getType().isSpecialBuilding()) {the_map.OnStaticBuildingDestroyed(unit);}
}

void FrogFish::onUnitMorph(Unit unit) {
    unit_storage.queue_store(unit);
    if (unit->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser) {
        the_map.OnGeyserNoticed(unit);
    }
}

void FrogFish::onUnitRenegade(Unit unit) {
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