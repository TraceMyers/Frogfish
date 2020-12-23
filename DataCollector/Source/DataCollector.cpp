#include "DataCollector.h"
#include <iostream>
#include <string>
#include <vector>

using namespace Filter;

void DataCollector::onStart() {
    Broodwar->sendText("Why hello there!");
    Broodwar->enableFlag(Flag::UserInput);
    Broodwar->setLocalSpeed(10);
    Broodwar->setCommandOptimizationLevel(2);
    onStart_alloc_debug_console();
    onStart_send_workers_to_mine();
    onStart_init_bwem_and_bweb();
}

void DataCollector::onFrame() {
	if (Broodwar->isPaused()) {return;}
}

void DataCollector::onSendText(std::string text) {
    Broodwar->sendText("%s", text.c_str());
}

void DataCollector::onReceiveText(BWAPI::Player player, std::string text) {
    Broodwar << player->getName() << " said \"" << text << "\"" << std::endl;
}

void DataCollector::onPlayerLeft(BWAPI::Player player) {
    Broodwar->sendText("Farewell %s!", player->getName().c_str());
}

void DataCollector::onNukeDetect(BWAPI::Position target) {
    // pass
}

void DataCollector::onUnitDiscover(BWAPI::Unit unit) {
    if (unit->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser) {
        the_map.OnGeyserNoticed(unit);
    }
    BWEB::Map::onUnitDiscover(unit);
}

void DataCollector::onUnitEvade(BWAPI::Unit unit) {
    // pass
}

void DataCollector::onUnitShow(BWAPI::Unit unit) {
    // pass
}

void DataCollector::onUnitHide(BWAPI::Unit unit) {
    // pass
}

void DataCollector::onUnitCreate(BWAPI::Unit unit) {
    if (unit->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser) {
        the_map.OnGeyserNoticed(unit);
    }
}

void DataCollector::onUnitDestroy(BWAPI::Unit unit) {
    if (unit->getType().isMineralField()) {the_map.OnMineralDestroyed(unit);}
    else if (unit->getType().isSpecialBuilding()) {the_map.OnStaticBuildingDestroyed(unit);}
    BWEB::Map::onUnitDestroy(unit);
}

void DataCollector::onUnitMorph(BWAPI::Unit unit) {
    if (unit->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser) {
        the_map.OnGeyserNoticed(unit);
    }
    BWEB::Map::onUnitMorph(unit);
}

void DataCollector::onUnitRenegade(BWAPI::Unit unit) {
    // pass
}

void DataCollector::onSaveGame(std::string gameName) {
    // pass
}

void DataCollector::onUnitComplete(Unit unit) {
    // pass
}

void DataCollector::onEnd(bool isWinner) {
    FreeConsole();
}