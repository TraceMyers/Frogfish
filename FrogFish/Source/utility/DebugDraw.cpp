#include "../FrogFish.h"
#include "DebugDraw.h"
#include "FrogMath.h"
#include "../production/MakeQueue.h"
#include "../production/BuildGraph.h"
#include "../production/BuildPlacement.h"
#include "../unitdata/UnitStorage.h"
#include "../unitdata/EnemyUnit.h"
#include "../unitdata/FrogUnit.h"
#include "../unitdata/EnemyUnit.h"
#include "../unitdata/BaseStorage.h"
#include "../unitdata/FrogBase.h"
#include "BWTimer.h"
#include <BWAPI.h>
#include <iostream>
#include <list>
#include <deque>

std::list<std::string *> out_buff;
BWTimer draw_timer;

void DebugDraw::append_debug_text(std::string *dbg_line) {
    if (out_buff.size() >= 18) {
        std::string *line = out_buff.front();
        delete line;
        out_buff.pop_front();
    }
    out_buff.push_back(dbg_line);
}

void DebugDraw::draw_debug_text() {
    std::list<std::string *>::const_iterator it;
    const int x = 10;
    int y = 10;
    for (it = out_buff.cbegin(); it != out_buff.cend(); ++it) {
        Broodwar->drawTextScreen(x, y, "%s", (**it).c_str());
        y += 16;
    }
}

void DebugDraw::draw_units(UnitStorage &unit_storage) {
    const std::map<int, FUnit> &self_units = unit_storage.self_units();
    register std::map<int, FUnit>::const_iterator self_it;
    register FUnit f_unit;
    FrogUnit::FTYPES f_type;
    BWAPI::Position pos;

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

void DebugDraw::draw_map() {
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

void DebugDraw::draw_base_info(BaseStorage &base_storage) {
    const std::vector<FBase> &self_bases = base_storage.get_self_bases();
    for (unsigned int i = 0; i < self_bases.size(); i++) {
        const FBase f_base = self_bases[i];
        const BWAPI::Position &center = f_base->get_center();
        Broodwar->drawBoxMap(
            BWAPI::Position(center.x + 7, center.y - 62),
            BWAPI::Position(center.x + 107, center.y - 9),
            BWAPI::Colors::Black,
            true 
        );
        Broodwar->drawTextMap(BWAPI::Position(center.x + 10, center.y - 62), "Frog Base!");
        Broodwar->drawTextMap(
            BWAPI::Position(center.x + 10, center.y - 52), 
            "structures: %d",
            f_base->get_structure_ct()
        );
        Broodwar->drawTextMap(
            BWAPI::Position(center.x + 10, center.y - 42), 
            "workers: %d",
            f_base->get_worker_ct()
        );
        Broodwar->drawTextMap(
            BWAPI::Position(center.x + 10, center.y - 32), 
            "resource depots: %d",
            f_base->get_resource_depot_ct()
        );
        Broodwar->drawTextMap(
            BWAPI::Position(center.x + 10, center.y - 22), 
            "larva: %d",
            f_base->get_larva_ct()
        );
    }
    const std::vector<EBase> &enemy_bases = base_storage.get_enemy_bases();
    for (unsigned int i = 0; i < enemy_bases.size(); i++) {
        const EBase e_base = enemy_bases[i];
        const BWAPI::Position &center = e_base->get_center();
        Broodwar->drawTextMap(BWAPI::Position(center.x + 10, center.y - 62), "Enemy Base!");
        Broodwar->drawTextMap(
            BWAPI::Position(center.x + 10, center.y - 52), 
            "structures: %d",
            e_base->get_structure_ct()
        );
        Broodwar->drawTextMap(
            BWAPI::Position(center.x + 10, center.y - 42), 
            "workers: %d",
            e_base->get_worker_ct()
        );
        Broodwar->drawTextMap(
            BWAPI::Position(center.x + 10, center.y - 32), 
            "resource depots: %d",
            e_base->get_resource_depot_ct()
        );
        Broodwar->drawTextMap(
            BWAPI::Position(center.x + 10, center.y - 22), 
            "larva: %d",
            e_base->get_larva_ct()
        );    
    }
}

void DebugDraw::draw_make_queue(MakeQueue &make_queue) {
    const std::deque<BWAPI::UnitType> &queue = make_queue.get_queue();
    int x = 10;
    int y = 10;
    for (auto &item : queue) {
        Broodwar->drawTextScreen(BWAPI::Position(x, y), "%s", item.getName().c_str());
        y += 10;
    }
}

void DebugDraw::draw_build_graphs() {
    BuildGraph *build_graphs = BuildPlacement::get_graphs();
    for (int i = 0; i < BuildPlacement::MAX_BASES; ++i) {
        BuildGraph &build_graph = build_graphs[i];
        if (build_graph.get_base() != nullptr) {
            auto &nodes = build_graph.get_nodes();
            for (unsigned int j = 0; j < nodes.size(); ++j) {
                if (nodes[j]->is_buildable()) {
                    BWAPI::TilePosition tp = nodes[j]->get_tilepos();
                    BWAPI::Position top_left(tp);
                    BWAPI::Position bot_right(BWAPI::TilePosition(tp.x + 1, tp.y + 1));
                    if (!nodes[j]->blocks_mining()) {
                        Broodwar->drawBoxMap(
                            top_left,
                            bot_right,
                            BWAPI::Colors::Green
                        );
                    }
                    else {
                        Broodwar->drawBoxMap(
                            top_left,
                            bot_right,
                            BWAPI::Colors::Purple
                        );
                    }
                    const std::vector<int> &dims = nodes[j]->get_buildable_dimensions();
                    Broodwar->drawTextMap(
                        BWAPI::Position(tp) + BWAPI::Position(5, 10), 
                        "%dx%d", dims[0], dims[1]
                    );
                }
            }
        }
    }
}

void DebugDraw::draw_bwem_data() {
    // seas and lakes
    BWAPI::WalkPosition walk_size = the_map.WalkSize();
    for (int i = 0; i < walk_size.x; ++i) {
        for (int j = 0; j < walk_size.y; ++j) {
            auto &minitile = the_map.GetMiniTile(BWAPI::WalkPosition(i, j));
            bool is_sea = minitile.Sea();
            bool is_lake = minitile.Lake();
            if (is_sea) {
                Broodwar->drawLineMap(
                    BWAPI::Position(i * 8, j * 8), 
                    BWAPI::Position(i * 8 + 8, j * 8 + 8),
                    BWAPI::Colors::Red
                );
            }
            if (is_lake) {
                Broodwar->drawLineMap(
                    BWAPI::Position(i * 8 + 8, j * 8),
                    BWAPI::Position(i * 8, j * 8 + 8),
                    BWAPI::Colors::Purple
                );
            }
        }
    }
    // chokepoints
    auto &areas = the_map.Areas();
    for (auto &area : areas) {
        auto &chokepoints = area.ChokePoints();
        for (auto &choke : chokepoints) {
            BWAPI::WalkPosition p = choke->Center();
            Broodwar->drawCircleMap(p.x, p.y, 3, BWAPI::Colors::Red);
            Broodwar->drawCircleMap(p.x * 8, p.y * 8, 3, BWAPI::Colors::Purple);
            const std::deque<BWAPI::WalkPosition> &geo = choke->Geometry();
            for (unsigned i = 0; i < geo.size() - 1; ++i) {
                Broodwar->drawLineMap(
                    BWAPI::Position(geo[i].x * 8, geo[i].y * 8),
                    BWAPI::Position(geo[i + 1].x * 8, geo[i + 1].y * 8),
                    BWAPI::Colors::Blue
                );
            }
        }
    }
}