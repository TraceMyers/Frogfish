#include "DataCollector.h"
#include "utility/BWTimer.h"
#include "Income.h"
#include <iostream>
#include <string>
#include <vector>

using namespace Filter;

BWTimer timer;
BWAPI::Player first = nullptr;
BWAPI::Player second = nullptr;
BWAPI::Player players[2];

void DataCollector::set_players() {
    auto player_set = Broodwar->getPlayers();

    for (auto &player : player_set) {
        if (player->getUnits().size() >= 5) {
            if (!player->isNeutral()) {
                if (first == nullptr) {
                    first = player;
                    players[0] = first;
                }
                else if (second == nullptr) {
                    second = player;
                    players[1] = second;
                }
            }
        }
    }
    printf("first player: %s\n", first->getName().c_str());
    printf("second player: %s\n", second->getName().c_str());
}

void DataCollector::onStart() {
    Broodwar->setLocalSpeed(4);
    //Broodwar->setCommandOptimizationLevel(2);
    onStart_alloc_debug_console();
    set_players();
    Income::init();
    timer.start(16, 0);
}

void DataCollector::onFrame() {
	if (Broodwar->isPaused()) {return;}

    timer.on_frame_update();

    Income::on_frame_update(players); 

    // test
    if (Income::ready() && timer.is_stopped()) {
        timer.restart();
        double *mps = Income::get_mps();
        double *gps = Income::get_gps();
        for (int i = 0; i < 2; ++i) {
            Broodwar->sendText("Player[%d] / mps: %.2lf / gps: %.2lf", i, mps[i], gps[i]);
        }
    }
}

void DataCollector::onSendText(std::string text) {
    Broodwar->sendText("%s", text.c_str());
}

void DataCollector::onReceiveText(BWAPI::Player player, std::string text) {
    Broodwar << player->getName() << " said \"" << text << "\"" << std::endl;
}

void DataCollector::onPlayerLeft(BWAPI::Player player) {
    // Broodwar->sendText("Farewell %s!", player->getName().c_str());
}

void DataCollector::onNukeDetect(BWAPI::Position target) {
    // pass
}

void DataCollector::onUnitDiscover(BWAPI::Unit unit) {
    // if (unit->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser) {
    //     the_map.OnGeyserNoticed(unit);
    // }
    // BWEB::Map::onUnitDiscover(unit);
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
    // if (unit->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser) {
    //     the_map.OnGeyserNoticed(unit);
    // }
}

void DataCollector::onUnitDestroy(BWAPI::Unit unit) {
    // if (unit->getType().isMineralField()) {the_map.OnMineralDestroyed(unit);}
    // else if (unit->getType().isSpecialBuilding()) {the_map.OnStaticBuildingDestroyed(unit);}
    // BWEB::Map::onUnitDestroy(unit);
}

void DataCollector::onUnitMorph(BWAPI::Unit unit) {
    // if (unit->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser) {
    //     the_map.OnGeyserNoticed(unit);
    // }
    // BWEB::Map::onUnitMorph(unit);
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