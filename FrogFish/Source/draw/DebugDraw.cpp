#include "DebugDraw.h"
#include "../data/UnitStorage.h"
#include "../data/EnemyUnit.h"
#include <BWAPI.h>
#include <iostream>
#include <string>
#include <list>

using namespace BWAPI;
using namespace Filter;

std::list<std::string *> out_buff;

void append_debug_text(std::string *dbg_line) {
    if (out_buff.size() >= 18) {
        std::string *line = out_buff.front();
        delete line;
        out_buff.pop_front();
    }
    out_buff.push_back(dbg_line);
}

void draw_debug_text() {
    std::list<std::string *>::const_iterator it;
    const int x = 10;
    int y = 10;
    for (it = out_buff.cbegin(); it != out_buff.cend(); ++it) {
        Broodwar->drawTextScreen(x, y, "%s", (**it).c_str());
        y += 16;
    }
}

void draw_units(UnitStorage &us) {
    std::list<BWAPI::Unit>& self_eggs = us.get_self_eggs();
    std::list<BWAPI::Unit>& self_larva = us.get_self_larva();
    std::list<BWAPI::Unit>& self_workers = us.get_self_workers();
    std::list<BWAPI::Unit>& self_army = us.get_self_army();
    std::list<BWAPI::Unit>& self_structures = us.get_self_structures();
    std::list<EnemyUnit>& enemy_workers = us.get_enemy_workers();
    std::list<EnemyUnit>& enemy_army = us.get_enemy_army();
    std::list<EnemyUnit>& enemy_structures = us.get_enemy_structures();
    std::list<std::list<BWAPI::Unit>> self_units {
        self_eggs, self_larva, self_workers, self_army, self_structures
    };
    std::list<std::list<EnemyUnit>> enemy_units {
        enemy_workers, enemy_army, enemy_structures
    };

    register std::list<std::list<BWAPI::Unit>>::iterator self_it;
    int i = 0;
    for (self_it = self_units.begin(); self_it != self_units.end(); ++self_it) {
        std::list<BWAPI::Unit>& cur_list = *self_it;
        register std::list<BWAPI::Unit>::iterator list_it;
        for (list_it = cur_list.begin(); list_it != cur_list.end(); ++list_it) {
            BWAPI::Unit& unit = *list_it;
            BWAPI::Position& pos = unit->getPosition();
            switch(i) {
                case 0:
                    Broodwar->drawTextMap(pos, "egg");
                break;
                case 1:
                    Broodwar->drawTextMap(pos, "larva");
                break;
                case 2:
                    Broodwar->drawTextMap(pos, "worker");
                break;
                case 3:
                    Broodwar->drawTextMap(pos, "army");
                break;
                default:
                    Broodwar->drawTextMap(pos, "struct");
            }
        }
        i++;
    }

    register std::list<std::list<EnemyUnit>>::iterator enemy_it;
    i = 0;
    for (enemy_it = enemy_units.begin(); enemy_it != enemy_units.end(); ++enemy_it) {
        std::list<EnemyUnit>& cur_list = *enemy_it;
        register std::list<EnemyUnit>::iterator list_it;
        for (list_it = cur_list.begin(); list_it != cur_list.end(); ++list_it) {
            EnemyUnit& unit = *list_it;
            const BWAPI::Position& pos = unit.getPosition();
            switch(i) {
                case 0:
                    Broodwar->drawTextMap(pos, "worker");
                break;
                case 1:
                    Broodwar->drawTextMap(pos, "army");
                break;
                default:
                    Broodwar->drawTextMap(pos, "struct");
            }
        }
        i++;
    }
}