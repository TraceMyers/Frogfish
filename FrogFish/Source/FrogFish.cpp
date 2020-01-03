#include "FrogFish.h"
#include "draw/DebugDraw.h"
#include "data/UnitStorage.h"
#include "utility/BWTimer.h"
#include <BWAPI.h>
#include <iostream>
#include <string>
#include <set>
#include <windows.h>
#include <stdio.h>

using namespace BWAPI;
using namespace Filter;

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

    for (BWAPI::Unit u : Broodwar->self()->getUnits()) {
        if (u->getType().isWorker()) {
            u->gather(u->getClosestUnit(IsMineralField));
        }
    }
    timer.start(5, 0, false);
}

void FrogFish::onFrame() {
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

void FrogFish::onUnitDestroy(BWAPI::Unit unit) {
    unit_storage.queue_remove(unit);
}

void FrogFish::onUnitMorph(BWAPI::Unit unit) {
    unit_storage.queue_store(unit);
}

void FrogFish::onUnitRenegade(BWAPI::Unit unit) {
    // pass
}

void FrogFish::onSaveGame(std::string gameName) {
    // pass
}

void FrogFish::onUnitComplete(BWAPI::Unit unit) {
    // broadcast for structure usage
}

void FrogFish::onEnd(bool isWinner) {
    FreeConsole();
    unit_storage.free_data();
}