#include "FrogFish.h"
#include "draw/DebugDraw.h"
#include "data/UnitStorage.h"
#include "data/BaseStorage.h"
#include "utility/BWTimer.h"
#include "data/EnemyBase.h"
#include "datamgmt/BaseOwnership.h"
#include "datamgmt/BaseAssets.h"
#include <BWAPI.h>
#include <iostream>
#include <string>
#include <set>
#include <chrono>

using namespace BWAPI;
using namespace Filter;

UnitStorage unit_storage;
BaseStorage base_storage;
BWTimer timer;

void FrogFish::onStart() {
    Broodwar->sendText("Hello Sailor!");
    Broodwar->enableFlag(Flag::UserInput);
    Broodwar->setLocalSpeed(21);
    Broodwar->setCommandOptimizationLevel(2);
    onStart_alloc_debug_console();
    onStart_send_workers_to_mine();
    onStart_init_bwem();
    init_base_storage(the_map, base_storage);
    //timer.start(20, 0, false);
}

void FrogFish::onFrame() {
	if (Broodwar->isReplay() || Broodwar->isPaused() || !Broodwar->self()) {
		return;
	}
    /*
    timer.on_frame_update();
    auto start_time = std::chrono::high_resolution_clock::now();
    */

    unit_storage.update();

    assign_new_bases(the_map, base_storage, unit_storage);
    assign_base_assets(the_map, base_storage, unit_storage);
    unassign_base_assets(the_map, base_storage, unit_storage);
    unassign_bases(base_storage);

    draw_units(unit_storage);
    draw_base_info(base_storage);

    unit_storage.clear_newly_assigned();

    /*
    auto end_time = std::chrono::high_resolution_clock::now();
    if (timer.is_stopped()) {
        auto time = end_time - start_time;
        std::cout << "one frame: " << time/std::chrono::milliseconds(1) << " ms." << std::endl;
        timer.restart();
    }
    */
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
}

void FrogFish::onUnitDestroy(Unit unit) {
    unit_storage.queue_remove(unit);
}

void FrogFish::onUnitMorph(Unit unit) {
    unit_storage.queue_store(unit);
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