#include "DebugDraw.h"
#include "BWTimer.h"
#include "../FrogFish.h"
#include "../basic/Units.h"
#include "../basic/Bases.h"
#include <BWAPI.h>
#include <iostream>
#include <list>
#include <deque>

using namespace Basic;

namespace Utility::DebugDraw {

namespace {
    std::list<std::string *> out_buff;
    BWTimer draw_timer;
}

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

void draw_units() {
    auto &self_units = Units::self_units();
    for (int i = 0; i < self_units.size(); ++i) {
        const BWAPI::Unit &unit = self_units[i];
        const Units::UnitData &unit_data = Units::data(unit);
        const Refs::UTYPE &u_type = unit_data.u_type;
        const BWAPI::Position &pos = unit_data.pos;
        switch(u_type) {
            case Refs::UTYPE::UNASSIGNED:
                Broodwar->drawTextMap(pos, "n/a");
                break;
            case Refs::UTYPE::EGG:
                Broodwar->drawTextMap(pos, "egg");
                break;
            case Refs::UTYPE::LARVA:
                Broodwar->drawTextMap(pos, "larva");
                break;
            case Refs::UTYPE::WORKER:
                Broodwar->drawTextMap(pos, "worker");
                break;
            case Refs::UTYPE::ARMY:
                Broodwar->drawTextMap(pos, "army");
                break;
            default:
                Broodwar->drawTextMap(pos, "struct");
        }
    }
    auto &enemy_units = Units::enemy_units();
    for (int i = 0; i < enemy_units.size(); ++i) {
        const BWAPI::Unit &unit = enemy_units[i];
        const Units::UnitData &unit_data = Units::data(unit);
        const Refs::UTYPE &u_type = unit_data.u_type;
        const BWAPI::Position &pos = unit_data.pos;
        switch(u_type) {
            case Refs::UTYPE::UNASSIGNED:
                Broodwar->drawTextMap(pos, "n/a");
                break;
            case Refs::UTYPE::EGG:
                Broodwar->drawTextMap(pos, "egg");
                break;
            case Refs::UTYPE::LARVA:
                Broodwar->drawTextMap(pos, "larva");
                break;
            case Refs::UTYPE::WORKER:
                Broodwar->drawTextMap(pos, "worker");
                break;
            case Refs::UTYPE::ARMY:
                Broodwar->drawTextMap(pos, "army");
                break;
            default:
                Broodwar->drawTextMap(pos, "struct");
        }

    }
}

void draw_bases() {
    auto &self_bases = Bases::self_bases();
    for (unsigned int i = 0; i < self_bases.size(); i++) {
        auto &base = self_bases[i];
        const BWAPI::Position &center = base->Center();
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
            Bases::structures(base).size()
        );
        Broodwar->drawTextMap(
            BWAPI::Position(center.x + 10, center.y - 42), 
            "workers: %d",
            Bases::workers(base).size()
        );
        Broodwar->drawTextMap(
            BWAPI::Position(center.x + 10, center.y - 32), 
            "resource depots: %d",
            Bases::depots(base).size()
        );
        Broodwar->drawTextMap(
            BWAPI::Position(center.x + 10, center.y - 22), 
            "larva: %d",
            Bases::larva(base).size()
        );
    }
    auto &enemy_bases = Bases::enemy_bases();
    for (unsigned int i = 0; i < enemy_bases.size(); i++) {
        auto &base = enemy_bases[i];
        const BWAPI::Position &center = base->Center();
        Broodwar->drawBoxMap(
            BWAPI::Position(center.x + 7, center.y - 62),
            BWAPI::Position(center.x + 107, center.y - 9),
            BWAPI::Colors::Black,
            true 
        );
        Broodwar->drawTextMap(BWAPI::Position(center.x + 10, center.y - 62), "Enemy Base!");
        Broodwar->drawTextMap(
            BWAPI::Position(center.x + 10, center.y - 52), 
            "structures: %d",
            Bases::structures(base).size()
        );
        Broodwar->drawTextMap(
            BWAPI::Position(center.x + 10, center.y - 42), 
            "workers: %d",
            Bases::workers(base).size()
        );
        Broodwar->drawTextMap(
            BWAPI::Position(center.x + 10, center.y - 32), 
            "resource depots: %d",
            Bases::depots(base).size()
        );
        Broodwar->drawTextMap(
            BWAPI::Position(center.x + 10, center.y - 22), 
            "larva: %d",
            Bases::larva(base).size()
        );    
    }
}

// void draw_map() {
//     try {
//         BWEM::utils::gridMapExample(the_map);
//         BWEM::utils::drawMap(the_map);
//         for (auto &area : the_map.Areas()) {
//             for (auto &geyser : area.Geysers()) {
//                 Broodwar->drawCircleMap(geyser->Unit()->getPosition(), 5, BWAPI::Colors::Red);
//             }
//         }
//     }
//     catch (const std::exception & e) {
//         Broodwar << "EXCEPTION: " << e.what() << std::endl;
//     }
// }


// void draw_make_queue(MakeQueue &make_queue) {
//     const std::deque<BWAPI::UnitType> &queue = make_queue.get_queue();
//     int x = 10;
//     int y = 10;
//     for (auto &item : queue) {
//         Broodwar->drawTextScreen(BWAPI::Position(x, y), "%s", item.getName().c_str());
//         y += 10;
//     }
// }

// void draw_build_graphs() {
//     BuildGraph *build_graphs = BuildPlacement::get_graphs();
//     for (int i = 0; i < BuildPlacement::MAX_BASES; ++i) {
//         BuildGraph &build_graph = build_graphs[i];
//         if (build_graph.get_base() != nullptr) {
//             auto &nodes = build_graph.get_nodes();
//             for (unsigned int j = 0; j < nodes.size(); ++j) {
//                 if (nodes[j]->is_buildable()) {
//                     BWAPI::TilePosition tp = nodes[j]->get_tilepos();
//                     BWAPI::Position top_left(tp);
//                     BWAPI::Position bot_right(BWAPI::TilePosition(tp.x + 1, tp.y + 1));
//                     if (!nodes[j]->blocks_mining()) {
//                         Broodwar->drawBoxMap(
//                             top_left,
//                             bot_right,
//                             BWAPI::Colors::Green
//                         );
//                     }
//                     else {
//                         Broodwar->drawBoxMap(
//                             top_left,
//                             bot_right,
//                             BWAPI::Colors::Purple
//                         );
//                     }
//                     const std::vector<int> &dims = nodes[j]->get_buildable_dimensions();
//                     Broodwar->drawTextMap(
//                         BWAPI::Position(tp) + BWAPI::Position(5, 10), 
//                         "%dx%d", dims[0], dims[1]
//                     );
//                 }
//             }
//         }
//     }
// }

// void draw_bwem_data() {
//     // seas and lakes
//     BWAPI::WalkPosition walk_size = the_map.WalkSize();
//     for (int i = 0; i < walk_size.x; ++i) {
//         for (int j = 0; j < walk_size.y; ++j) {
//             auto &minitile = the_map.GetMiniTile(BWAPI::WalkPosition(i, j));
//             bool is_sea = minitile.Sea();
//             bool is_lake = minitile.Lake();
//             if (is_sea) {
//                 Broodwar->drawLineMap(
//                     BWAPI::Position(i * 8, j * 8), 
//                     BWAPI::Position(i * 8 + 8, j * 8 + 8),
//                     BWAPI::Colors::Red
//                 );
//             }
//             if (is_lake) {
//                 Broodwar->drawLineMap(
//                     BWAPI::Position(i * 8 + 8, j * 8),
//                     BWAPI::Position(i * 8, j * 8 + 8),
//                     BWAPI::Colors::Purple
//                 );
//             }
//         }
//     }
//     // chokepoints
//     auto &areas = the_map.Areas();
//     for (auto &area : areas) {
//         auto &chokepoints = area.ChokePoints();
//         for (auto &choke : chokepoints) {
//             BWAPI::WalkPosition p = choke->Center();
//             Broodwar->drawCircleMap(p.x, p.y, 3, BWAPI::Colors::Red);
//             Broodwar->drawCircleMap(p.x * 8, p.y * 8, 3, BWAPI::Colors::Purple);
//             const std::deque<BWAPI::WalkPosition> &geo = choke->Geometry();
//             for (unsigned i = 0; i < geo.size() - 1; ++i) {
//                 Broodwar->drawLineMap(
//                     BWAPI::Position(geo[i].x * 8, geo[i].y * 8),
//                     BWAPI::Position(geo[i + 1].x * 8, geo[i + 1].y * 8),
//                     BWAPI::Colors::Blue
//                 );
//             }
//         }
//     }
// }

}