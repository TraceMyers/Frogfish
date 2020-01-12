#include "DebugDraw.h"
#include "../production/MakeQueue.h"
#include "../data/UnitStorage.h"
#include "../data/EnemyUnit.h"
#include "../data/FrogUnit.h"
#include "../data/EnemyUnit.h"
#include "../data/BaseStorage.h"
#include "../data/FrogBase.h"
#include <BWAPI.h>
#include <iostream>
#include <list>
#include <deque>

using namespace BWAPI;

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

void draw_units(UnitStorage &unit_storage) {
    const std::map<int, FUnit> &self_units = unit_storage.self_units();
    register std::map<int, FUnit>::const_iterator self_it;
    register FUnit f_unit;
    FrogUnit::FTYPES f_type;
    Position pos;

    for (self_it = self_units.begin(); self_it != self_units.end(); ++self_it) {
        f_unit = self_it->second;
        f_type = f_unit->f_type;
        pos = f_unit->get_pos();
        switch(f_type) {
            case FrogUnit::FTYPES::EGG:
                Broodwar->drawTextMap(pos, "egg");
            break;
            case FrogUnit::FTYPES::LARVA:
                Broodwar->drawTextMap(pos, "larva");
            break;
            case FrogUnit::FTYPES::WORKER:
                Broodwar->drawTextMap(pos, "worker");
            break;
            case FrogUnit::FTYPES::ARMY:
                Broodwar->drawTextMap(pos, "army");
            break;
            default:
                Broodwar->drawTextMap(pos, "struct");
        }
    }

    const std::map<int, EUnit> &enemy_units = unit_storage.enemy_units();
    register std::map<int, EUnit>::const_iterator enemy_it;
    register EUnit e_unit;
    EnemyUnit::ETYPE e_type;

    for (enemy_it = enemy_units.begin(); enemy_it != enemy_units.end(); ++enemy_it) {
        e_unit  = enemy_it->second;
        e_type = e_unit->e_type;
        pos = e_unit->get_pos();

        switch(e_type) {
            case 0:
                Broodwar->drawTextMap(pos, "worker");
            break;
            case 1:
                Broodwar->drawTextMap(pos, "army");
            break;
            case 2:
                Broodwar->drawTextMap(pos, "struct");
            break;
            default:
                Broodwar->drawTextMap(pos, "unknown");
        }
    }
}

void draw_map(BWEM::Map &the_map) {
    try {
        BWEM::utils::gridMapExample(the_map);
        BWEM::utils::drawMap(the_map);
        for (auto &area : the_map.Areas()) {
            for (auto &geyser : area.Geysers()) {
                Broodwar->drawCircleMap(geyser->Unit()->getPosition(), 5, BWAPI::Colors::Red);
            }
        }
    }
    catch (const std::exception & e) {
        Broodwar << "EXCEPTION: " << e.what() << std::endl;
    }
}

void draw_base_info(BaseStorage &base_storage) {
    const std::vector<FBase> &self_bases = base_storage.get_self_bases();
    for (unsigned int i = 0; i < self_bases.size(); i++) {
        const FBase f_base = self_bases[i];
        const Position &center = f_base->get_center();
        Broodwar->drawBoxMap(
            Position(center.x + 7, center.y - 62),
            Position(center.x + 107, center.y - 9),
            BWAPI::Colors::Black,
            true 
        );
        Broodwar->drawTextMap(Position(center.x + 10, center.y - 62), "Frog Base!");
        Broodwar->drawTextMap(
            Position(center.x + 10, center.y - 52), 
            "structures: %d",
            f_base->get_structure_ct()
        );
        Broodwar->drawTextMap(
            Position(center.x + 10, center.y - 42), 
            "workers: %d",
            f_base->get_worker_ct()
        );
        Broodwar->drawTextMap(
            Position(center.x + 10, center.y - 32), 
            "resource depots: %d",
            f_base->get_resource_depot_ct()
        );
        Broodwar->drawTextMap(
            Position(center.x + 10, center.y - 22), 
            "larva: %d",
            f_base->get_larva_ct()
        );
    }
    const std::vector<EBase> &enemy_bases = base_storage.get_enemy_bases();
    for (unsigned int i = 0; i < enemy_bases.size(); i++) {
        const EBase e_base = enemy_bases[i];
        const Position &center = e_base->get_center();
        Broodwar->drawTextMap(Position(center.x + 10, center.y - 62), "Enemy Base!");
        Broodwar->drawTextMap(
            Position(center.x + 10, center.y - 52), 
            "structures: %d",
            e_base->get_structure_ct()
        );
        Broodwar->drawTextMap(
            Position(center.x + 10, center.y - 42), 
            "workers: %d",
            e_base->get_worker_ct()
        );
        Broodwar->drawTextMap(
            Position(center.x + 10, center.y - 32), 
            "resource depots: %d",
            e_base->get_resource_depot_ct()
        );
        Broodwar->drawTextMap(
            Position(center.x + 10, center.y - 22), 
            "larva: %d",
            e_base->get_larva_ct()
        );    
    }
}
void draw_make_queue(MakeQueue &make_queue) {
    const std::deque<BWAPI::UnitType> &queue = make_queue.get_queue();
    int x = 10;
    int y = 10;
    for (auto &item : queue) {
        Broodwar->drawTextScreen(BWAPI::Position(x, y), "%s", item.getName().c_str());
        y += 10;
    }
}