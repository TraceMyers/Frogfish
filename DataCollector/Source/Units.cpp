#include "Units.h"


namespace Units {

    namespace {

        BWAPI::Race races[2];
        int counts[2][TERRAN_TYPE_CT] {0};
        bool can_make[2][TERRAN_TYPE_CT] {false};

    }

    void init(const BWAPI::Player *players) {
        races[0] = players[0]->getRace();
        races[1] = players[1]->getRace();
    }


    void on_frame_update(const BWAPI::Player *players) {
        for (int i = 0; i < 2; ++i) {
            const BWAPI::Unitset &units = players[i]->getUnits();
            if (races[i] == BWAPI::Races::Zerg) {
                for (int j = 0; j < ZERG_TYPE_CT; ++j) {
                    counts[i][j] = 0;
                    for (auto & unit : units) {
                        BWAPI::UnitType type = unit->getType();
                        if (type == ZERG_TYPES[j]) { 
                            counts[i][j] += 1; 
                        }
                    }
                    can_make[i][j] = players[i]->isUnitAvailable(ZERG_TYPES[j]);
                }
            }
            else if (races[i] == BWAPI::Races::Terran) {
                for (int j = 0; j < TERRAN_TYPE_CT; ++j) {
                    counts[i][j] = 0;
                    for (auto & unit : units) {
                        BWAPI::UnitType type = unit->getType();
                        if (type == TERRAN_TYPES[j]) { 
                            counts[i][j] += 1; 
                        }
                    }
                    can_make[i][j] = players[i]->isUnitAvailable(TERRAN_TYPES[j]);
                }
            }
            else {
                for (int j = 0; j < PROTOSS_TYPE_CT; ++j) {
                    counts[i][j] = 0;
                    for (auto & unit : units) {
                        BWAPI::UnitType type = unit->getType();
                        if (type == PROTOSS_TYPES[j]) { 
                            counts[i][j] += 1; 
                        }
                    }
                    can_make[i][j] = players[i]->isUnitAvailable(PROTOSS_TYPES[j]);
                }
            }
            
        }
    }

    int* get_unit_type_counts(int player_index) {
        return counts[player_index];
    }

    bool* get_can_make(int player_index) {
        return can_make[player_index];
    }

}