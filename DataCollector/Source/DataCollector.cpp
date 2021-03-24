#include "DataCollector.h"
#include "utility/BWTimer.h"
#include "Income.h"
#include "Units.h"
#include <iostream>
#include <string>
#include <vector>

using namespace Filter;

BWTimer timer;
BWAPI::Player first = nullptr;
BWAPI::Player second = nullptr;
BWAPI::Player players[2];
BWAPI::Race   races[2];

// NLP model rough idea:
    // every unit type + upgrade + tech set is a unique word
    // existence of tech requirements constantly held in model memory
    // when units are made by self, their words get added to the text in order
    // when unique enemy units are seen, their words get added as soon as seen
    // unit deaths also each a word, but don't care about upgrades
    // output limited to those unique zerg words (only moving units + hatcheries)
    // uses last word + memory

void DataCollector::set_players() {
    auto player_set = Broodwar->getPlayers();

    for (auto &player : player_set) {
        if (player->getUnits().size() >= 5) {
            if (!player->isNeutral()) {
                if (first == nullptr) {
                    first = player;
                    players[0] = first;
                    races[0] = first->getRace();
                }
                else if (second == nullptr) {
                    second = player;
                    players[1] = second;
                    races[1] = second->getRace();
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
    Units::init(players);
    timer.start(30, 0);
}

void DataCollector::onFrame() {
	if (Broodwar->isPaused()) {return;}

    timer.on_frame_update();

    Income::on_frame_update(players); 
    Units::on_frame_update(players);

    // test
    if (Income::ready() && timer.is_stopped()) {
        timer.restart();
        double *mps = Income::get_mps();
        double *gps = Income::get_gps();
        for (int i = 0; i < 2; ++i) {
            Broodwar->sendText("Player[%d] / mps: %.2lf / gps: %.2lf", i, mps[i], gps[i]);
        }
        for (int i = 0; i < 2; ++i) {
            printf("player [%s] counts: \n", players[i]->getName().c_str());
            int* counts = Units::get_unit_type_counts(i);
            int type_ct;
            const BWAPI::UnitType *types;
            if (races[i] == BWAPI::Races::Zerg) { 
                type_ct = Units::ZERG_TYPE_CT; 
                types = Units::ZERG_TYPES;
            }
            else if (races[i] == BWAPI::Races::Terran) { 
                type_ct = Units::TERRAN_TYPE_CT; 
                types = Units::TERRAN_TYPES;
            }
            else { 
                type_ct = Units::PROTOSS_TYPE_CT; 
                types = Units::PROTOSS_TYPES;
            }
            for (int j = 0; j < type_ct; ++j) {
                const BWAPI::UnitType &type = types[j];
                if (!type.isBuilding()) {
                    printf("\t %s: %d\n", type.c_str(), counts[j]);
                }
            }
        }
        printf("\n");
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