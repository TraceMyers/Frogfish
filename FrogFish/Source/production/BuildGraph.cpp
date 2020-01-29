#include "BuildGraph.h"
#include "../utility/FrogMath.h"
#include <BWAPI.h>
#include <BWEM/bwem.h>

using namespace Basic;

namespace BuildGraph {
////////////////////////////////////////+/////////////////////////////////////////
///                                 BuildNode                                   //
//////////////////////////////////////////////////////////////////////////////////
namespace {
    typedef struct BuildNode {
        const BWEM::Tile&        tile;
        BWAPI::TilePosition      tilepos;
        BWAPI::Position          pos;
        std::vector<BuildNode *> edges;
        std::vector<int>         buildable_dimensions;
        bool                     blocks_mining;
        bool                     blocks_larva;
        bool                     occupied;
        int                      id;

        BuildNode(const BWEM::Tile &_t, BWAPI::TilePosition tp, int _id) :
        tile(_t),
        tilepos(tp),
        id(_id)
        {}
        void set_buildable_dimensions(int x, int y) {
            buildable_dimensions[0] = x;
            buildable_dimensions[1] = y;
        }
        BuildNode *get_edge(int dir) {return edges[dir];}
        bool operator ==(const BuildNode &other) const;
    } *BNode;
}
////////////////////////////////////////+/////////////////////////////////////////
///                                BuildGraph                                   //
//////////////////////////////////////////////////////////////////////////////////

// TODO: to scan for nodes that can build out, look for nodes missing edges, rather than
// trying ot build out from just any node. Do this at one base per frame

namespace {
    static const int    MAX_BASES = 30;
    std::vector<BNode>  build_nodes[MAX_BASES];
    std::vector<BNode>  geyser_nodes[MAX_BASES];

    int                 base_ct = 0;
    int                 node_ID_counter = 0;
    std::vector<BNode>  remove_queue {nullptr};

    std::vector<double> resource_blocking_angles[MAX_BASES];
    enum                DIRECTIONS {RIGHT, UP, LEFT, DOWN};

    const int           CHUNK_SIZE = 30;
    int                 start_chunk[MAX_BASES] {0};
    int                 end_chunk[MAX_BASES] {CHUNK_SIZE};

    std::vector<double> get_buffered_resource_angles(const BWEM::Base *base) {
        std::vector<double> return_angles {-1, -1, -1}; 
        auto &minerals = base->Minerals();
        if (minerals.size() <= 1 || Bases::depots(base).size() == 0) {
            return return_angles;
        }
        BWAPI::Position hatch_center = base->Center();
        hatch_center.x += 64;
        hatch_center.y += 48;
        std::vector<double> angles;

        auto &geysers = base->Geysers();
        if (geysers.size() > 0) {
            for (auto & geyser : geysers) {
                if (geyser->Pos().getApproxDistance(hatch_center) < 400) {
                    double geyser_angle = FrogMath::get_angle(hatch_center, geyser->Pos());
                    angles.push_back(geyser_angle);
                } 
            }
        }
        for (auto &mineral : minerals) {
            if (mineral->Pos().getApproxDistance(hatch_center) < 300) {
                angles.push_back(FrogMath::get_angle(hatch_center, mineral->Pos()));
            }
        }
        for (unsigned int i = 0; i < angles.size() - 1; ++i) {
            for (unsigned int j = i + 1; j < angles.size(); ++j) {
                if (angles[j] < angles[i]) {
                    double temp = angles[i];
                    angles[i] = angles[j];
                    angles[j] = temp;
                }
            }
        }

        double sub_value = 0.0;
        std::vector<double> saved_angles;
        for (double element : angles) {
            saved_angles.push_back(element);
        }
        bool found_gap = false;
        for (unsigned int i = 1; i < angles.size(); ++i) {
            if (!found_gap && angles[i] - angles[i - 1] > 2.8) {
                sub_value = angles[i];
                found_gap = true;
                for (unsigned int j = 0; j < i; ++j) {
                    angles[j] += 2 * FrogMath::FM_PI - sub_value;
                }
            }
            if (found_gap) {
                angles[i] -= sub_value;
            }
        }

        int max_i = std::distance(angles.begin(), std::max_element(angles.begin(), angles.end()));
        int min_i = std::distance(angles.begin(), std::min_element(angles.begin(), angles.end()));
        int inside_i = 0;
        for (unsigned i = 0; i < angles.size(); ++i) {
            if (i != max_i && i != min_i) {
                inside_i = i;
                break;
            }
        }
        return_angles[0] = saved_angles[min_i] - 0.4;
        if (return_angles[0] < 0) {
            return_angles[0] += 2 * FrogMath::FM_PI;
        }
        return_angles[1] = saved_angles[max_i] + 0.4;
        if (return_angles[1] > 2 * FrogMath::FM_PI) {
            return_angles[1] -= 2 * FrogMath::FM_PI;
        }
        return_angles[2] = saved_angles[inside_i];
        return return_angles;
    }

    void seed_creep(int base_index, const BWAPI::TilePosition &tilepos) {
        const BWEM::Tile &_t = the_map.GetTile(tilepos);
        if (
            tilepos.isValid() 
            && _t.Walkable()
            && Broodwar->hasCreep(tilepos)
            && find_node_at(base_index, tilepos) == nullptr
        ) {
            build_nodes[base_index].push_back(new BuildNode(_t, tilepos, node_ID_counter));
            ++node_ID_counter;
        }
    }

    bool node_near_hatch(BNode node, std::vector<BWAPI::TilePosition> hatch_tilepositions) {
        TilePosition &node_tilepos = node->tilepos;
        int path[16][2] {
            {0, -3}, {1, 0}, {1, 0}, {0, 1}, 
            {0, 1}, {0, 1}, {0, -3}, {-3, 0}, 
            {-1, 0}, {-1, 0}, {-1, 0}, {0, 1}, 
            {0, 1}, {0, 1}, {0, 1}
        };
        for (int i = 0; i < 16; ++i) {
            node_tilepos.x += path[i][0];
            node_tilepos.y += path[i][1];
            for (auto &hatch_tp : hatch_tilepositions) {
                if (hatch_tp == node_tilepos) {
                    node->blocks_larva = true;
                    return true;
                }
            }
        } 
        node->blocks_larva = false;
        return false;
    }

    void update_chunk(int base_index) {
        auto &base = Bases::all_bases()[base_index];
        for (int i = start_chunk[base_index]; i < end_chunk[base_index]; ++i) {
            BNode check_node = build_nodes[base_index][i];
            int disconnected_ct = 0;
            bool this_node_mineral_blocking = check_node->blocks_mining;
            
            for (auto &edge : check_node->edges) {
                if (edge == nullptr) {
                    ++disconnected_ct;
                }
            }
            if (disconnected_ct == 4 || !Broodwar->hasCreep(check_node->tilepos)) {
                remove_queue.push_back(check_node);
                continue;
            }

            check_node->buildable_dimensions[0] = 0;
            auto &hatches = Bases::depots(base);
            std::vector<BWAPI::TilePosition> hatch_tilepositions;
            for (auto &hatch : hatches) {
                hatch_tilepositions.push_back(Units::data(hatch).tilepos);
            }
            bool below_hatch = node_near_hatch(check_node, hatch_tilepositions);
            if (below_hatch) {
                continue;
            }

            BNode bnode = check_node->get_edge(RIGHT);
            int j;
            if (bnode != nullptr) {
                int buildable_size_path[11] {
                    LEFT, DOWN, RIGHT, RIGHT, UP, RIGHT, DOWN, DOWN, LEFT, LEFT, LEFT
                };
                for (j = 0; j < 12; ++j) {
                    const TilePosition tp = bnode->tilepos;
                    if (
                        !Broodwar->isBuildable(tp, true) 
                        ||
                        (!this_node_mineral_blocking && bnode->blocks_mining)
                        ||
                        bnode->blocks_larva
                    ) {
                        break;
                    }
                    if (j == 3) {
                        check_node->set_buildable_dimensions(2, 2);
                    }
                    else if (j == 5) {
                        check_node->set_buildable_dimensions(3, 2);
                    }
                    else if (j == 7) {
                        check_node->set_buildable_dimensions(4, 2);
                    }
                    else if (j == 11) {
                        check_node->set_buildable_dimensions(4, 3);
                        break;
                    }
                    bnode = bnode->get_edge(buildable_size_path[j]);
                    if (bnode == nullptr) {
                        break;
                    }
                }
            }
        }
    }

    void try_expand(int base_index) {
        for (int i = start_chunk[base_index]; i < end_chunk[base_index]; ++i) {
            const BNode &cur_node = build_nodes[base_index][i];
            const BWAPI::TilePosition &tp = cur_node->tilepos;
            if (Broodwar->isVisible(tp)) {
                int cardinal[4][2] {{1, 0}, {0, -1}, {-1, 0}, {0, 1}};
                for (int j = 0; j < 4; ++j) {
                    if (cur_node->edges[j] == nullptr) {
                        BWAPI::TilePosition check_tp(tp.x + cardinal[j][0], tp.y + cardinal[j][1]);
                        if (
                            the_map.Valid(check_tp)
                            && the_map.GetTile(check_tp).Walkable()
                            && Broodwar->hasCreep(check_tp)
                        ) {
                            BNode check_node = find_node_at(base_index, check_tp);
                            if (check_node == nullptr) {
                                const BWEM::Tile &t = the_map.GetTile(check_tp);
                                check_node = new BuildNode(t, check_tp, node_ID_counter);
                                build_nodes[base_index].push_back(check_node);
                                ++node_ID_counter;
                            }
                            cur_node->edges[j] = check_node;
                            switch(j) {
                                case RIGHT: // 0
                                    check_node->edges[LEFT] = cur_node;
                                    break;
                                case UP:    // 1
                                    check_node->edges[DOWN] = cur_node;
                                    break;
                                case LEFT:  // 2
                                    check_node->edges[RIGHT] = cur_node;
                                    break;
                                case DOWN:  // 3
                                    check_node->edges[UP] = cur_node;
                            }
                        }
                    }
                }
            }
        }
    }

    void flag_resource_blocking_nodes(int base_index) {
        BWAPI::Position base_center = Bases::all_bases()[base_index]->Center();
        for (int i = start_chunk[base_index]; i < end_chunk[base_index]; ++i) {
            auto &node = build_nodes[base_index][i];
            double angle_to_node = FrogMath::get_angle(base_center, node->pos);
            if (FrogMath::angle_is_between(
                angle_to_node,
                resource_blocking_angles[base_index][0],
                resource_blocking_angles[base_index][1],
                resource_blocking_angles[base_index][2]
            )) {
                node->blocks_mining = true;
            }
            else {
                node->blocks_mining = false;
            }
        }
    }

    BNode find_node_at(int base_index, const TilePosition &tilepos) {
        if (tilepos.isValid()) {
            auto node_it = find_if(
                build_nodes[base_index].begin(), 
                build_nodes[base_index].end(), 
                [tilepos]
                    (const BNode &bn)
                    {return bn->tilepos == tilepos;}
            );
            if (node_it != build_nodes[base_index].end()) {
                return *node_it;
            }
        }
        return nullptr;
    }

    void remove_dead_nodes() {
        BNode edge;
        for (int i = 0; i < base_ct; ++i) {
            for (auto node : remove_queue) {
                if((edge = node->get_edge(RIGHT)) != nullptr) edge->edges[LEFT] = nullptr;
                if((edge = node->get_edge(UP)) != nullptr) edge->edges[DOWN] = nullptr;
                if((edge = node->get_edge(LEFT)) != nullptr) edge->edges[RIGHT] = nullptr;
                if((edge = node->get_edge(DOWN)) != nullptr) edge->edges[UP] = nullptr;
                auto node_it = std::remove(build_nodes[i].begin(), build_nodes[i].end(), node);
                if (node_it != build_nodes[i].end()) {
                    build_nodes[i].erase(node_it);
                }
                delete node;
            }
        }
        remove_queue.clear();
    }

}

void init() {
    auto &all_bases = Bases::all_bases();
    base_ct = all_bases.size();
    for (int i = 0; i < base_ct; ++i) {
        resource_blocking_angles[i] = get_buffered_resource_angles(all_bases[i]);
    }
}

void on_frame_update() {
    auto &bases = Bases::all_bases();
    for (int i = 0; i < base_ct; ++i) {
        const BWEM::Base *base = bases[i];
        for (auto &structure : Bases::structures(base)) {
            const Units::UnitData &unit_data = Units::data(structure);
            seed_creep(i, unit_data.tilepos);
        }
        int base_nodes_sz = build_nodes[i].size();
        if (base_nodes_sz > 0) {
            if (end_chunk[i] > base_nodes_sz) {
                if (start_chunk[i] >= base_nodes_sz) {
                    start_chunk[i] = 0;
                    if (CHUNK_SIZE >= base_nodes_sz) {
                        end_chunk[i] = base_nodes_sz;
                    }
                    else {
                        end_chunk[i] = CHUNK_SIZE;
                    }
                }
                else {
                    end_chunk[i] = base_nodes_sz;
                }
            }
            update_chunk(i);
            try_expand(i);
            if (resource_blocking_angles[i][0] >= 0) {
                // TODO: move this out of the on_frame loop (?)
                flag_resource_blocking_nodes(i);
            }
            start_chunk[i] += CHUNK_SIZE;
            end_chunk[i] += CHUNK_SIZE;
        }
    }
    remove_dead_nodes();
}

void free_data() {
    for (int i = 0; i < base_ct; ++i) {
        const std::vector<BNode> &base_nodes = build_nodes[i];
        for (int j = 0; j < base_nodes.size(); ++j) {
            delete base_nodes[j];
        }
        build_nodes[i].clear();
    }
}

}