#include "FrogFish.h"
#include "utility/BWTimer.h"
#include "utility/DebugDraw.h"
#include "basic/Units.h"
#include "basic/Bases.h"
#include "basic/Tech.h"
#include <BWAPI.h>
#include <iostream>
#include <string>
#include <vector>

using namespace Filter;
// TODO:
//  - near total refactor with neat namespaces, better task organization; generally
//  in need of an organization style
//      - DELETE 500+ lines
//  - need to expand the features of pathfinding beyond simply providing paths.
//  - pathfinding needs to solve problems like: getting a unit from point a
//  to point b while avoiding potential and known danger. This would mean things like
//  risk-averse pathing, (moderately well-)known & seen danger avoidance, and
//  connecting with battle sim to decide if any path between a and b is too dangerous
//  for the unit to attempt without army intervention
//  - For now, pathing just needs to handle moving units from a to b with either
//  attacking or not attacking in mind

BWTimer timer;

// todo print fish puns every now and again

void FrogFish::onStart() {
    Broodwar->sendText("Hello Sailor!");
    Broodwar->enableFlag(Flag::UserInput);
    Broodwar->setLocalSpeed(12);
    Broodwar->setCommandOptimizationLevel(2);
    onStart_alloc_debug_console();
    onStart_send_workers_to_mine();
    onStart_init_bwem_and_bweb();
    Basic::Bases::init();
    // init build placement
    // init prod coord
    // load build order
    timer.start(1,0);
}

void FrogFish::onFrame() {
	if (Broodwar->isReplay() || Broodwar->isPaused() || !Broodwar->self()) {return;}
    timer.on_frame_update();

    // 1. update basic data that everything else references
    // clear base storage stuff
    Basic::Units::on_frame_update();
    Basic::Bases::on_frame_update();
    Basic::Tech::on_frame_update();

    // 2. update production data
    // update production coordinator

    // 3. issue commands that require newly assigned lists
    // send mineral workers to gas

    // PRE-LAST. draw
    Utility::DebugDraw::draw_units();
    Utility::DebugDraw::draw_bases();
    // DebugDraw::draw_build_graphs();

    if (Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0) {return;}
    // RUN COMMANDS -----------------------------------------------------------------
    // produce (needs to move up)
    // send idle workers to mine minerals (needs to move up)
}

void FrogFish::onSendText(std::string text) {
    Broodwar->sendText("%s", text.c_str());
}

void FrogFish::onReceiveText(BWAPI::Player player, std::string text) {
    Broodwar << player->getName() << " said \"" << text << "\"" << std::endl;
}

void FrogFish::onPlayerLeft(BWAPI::Player player) {
    Broodwar->sendText("Farewell %s!", player->getName().c_str());
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
    if (unit->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser) {
        the_map.OnGeyserNoticed(unit);
    }
    else {
        Basic::Units::queue_store(unit);
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
    Basic::Units::queue_store(unit);
    if (unit->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser) {
        Basic::Units::queue_remove(unit);
        the_map.OnGeyserNoticed(unit);
    }
}

void FrogFish::onUnitDestroy(BWAPI::Unit unit) {
    Basic::Units::queue_remove(unit);
    if (unit->getType().isMineralField()) {the_map.OnMineralDestroyed(unit);}
    else if (unit->getType().isSpecialBuilding()) {the_map.OnStaticBuildingDestroyed(unit);}
    BWEB::Map::onUnitDestroy(unit);
}

void FrogFish::onUnitMorph(BWAPI::Unit unit) {
    if (unit->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser) {
        Basic::Units::queue_remove(unit);
        the_map.OnGeyserNoticed(unit);
    }
    else {
        Basic::Units::queue_store(unit);
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
    // free unit data
    // free base data
}