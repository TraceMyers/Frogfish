#include "FrogFish.h"
#include "draw/DebugDraw.h"
#include "data/UnitStorage.h"
#include "utility/BWTimer.h"
#include <BWAPI.h>
#include <BWEM/bwem.h>
#include <iostream>
#include <string>
#include <set>
#include <windows.h>
#include <stdio.h>

using namespace BWAPI;
using namespace Filter;

namespace {auto & the_map = BWEM::Map::Instance();}

UnitStorage unit_storage;
BWTimer timer;

void FrogFish::onStart() {
    Broodwar->sendText("Hello Sailor!");
    Broodwar->enableFlag(Flag::UserInput);
    Broodwar->setLocalSpeed(21);
    Broodwar->setCommandOptimizationLevel(2);

    // init debug output console
    FILE *pFile = nullptr;
    AllocConsole();
    freopen_s(&pFile, "CONOUT$", "w", stdout);

    for (Unit u : Broodwar->self()->getUnits()) {
        if (u->getType().isWorker()) {
            u->gather(u->getClosestUnit(IsMineralField));
        }
    }

    try {
        Broodwar << "Map init..." << std::endl;
        the_map.Initialize();
        bool starting_locs_ok = the_map.FindBasesForStartingLocations();
        assert(starting_locs_ok);
        printf("Base ct: %d\n", the_map.BaseCount());
    }
    catch (const std::exception e) {
        Broodwar << "EXCEPTION: " << e.what() << std::endl;
    }
}

void FrogFish::onFrame() {
	if (Broodwar->isReplay() || Broodwar->isPaused() || !Broodwar->self()) {
		return;
	}

    unit_storage.store_queued();
    unit_storage.remove_queued();
    unit_storage.update_self_units();
    unit_storage.update_enemy_units();

    draw_units(unit_storage);

    timer.on_frame_update();
    if (timer.is_stopped()) {
        timer.restart();
    }
    
    // print_debug_text();
    unit_storage.clear_newly_assigned();
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
    /*
    if (unit->getPlayer() == Broodwar->self()) {
        // send new self unit to on-screen debug buffer
        std::string *str_ptr = new std::string("new: ");
        (*str_ptr).append(unit->getType().getName().c_str());
        append_debug_text(str_ptr);
    }
    */
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
}