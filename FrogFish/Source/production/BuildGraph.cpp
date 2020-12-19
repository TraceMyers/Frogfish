#include "BuildGraph.h"
#include "../FrogFish.h"
#include "../utility/FrogMath.h"
#include "../basic/Bases.h"
#include "../basic/Units.h"

using namespace Basic;

namespace Production::BuildGraph {

// TODO: to scan for nodes that can build out, look for nodes missing edges, rather than
// trying to build out from just any node. Do this at one base per frame
// TODO: inventory scanning algorithm from Unreal first person game helpful?
// TODO: create more effective system for stopping units from building between hatch & resources;
//       allow for exceptions in special circumstances
// TODO: use 'occupied' status for nodes
// TODO: unreserve site function

    namespace {
        static const int    MAX_BASES = 30;
        std::vector<BNode>  _build_nodes[MAX_BASES];
        std::vector<BNode>  geyser_nodes[MAX_BASES];

        int                 base_ct = 0;
        int                 node_ID_counter = 0;
        std::vector<std::pair<BNode,int>>  remove_queue;

        std::vector<double> resource_blocking_angles[MAX_BASES];
        enum                DIRECTIONS {RIGHT, UP, LEFT, DOWN};

        const int           CHUNK_SIZE = 30;
        int                 start_chunk[MAX_BASES] {0};
        int                 end_chunk[MAX_BASES] {CHUNK_SIZE};

        int                 reservation_ID_counter = 0;

        void set_resource_blocking_angles(int base_index) {
            const BWEM::Base *base = Bases::all_bases()[base_index];
            auto &minerals = base->Minerals();
            if (minerals.size() <= 1 || Bases::depots(base).size() == 0) {
                return;
            }
            BWAPI::Position hatch_center = base->Center();
            hatch_center.x += 64;
            hatch_center.y += 48;
            std::vector<double> angles;

            auto &geysers = base->Geysers();
            if (geysers.size() > 0) {
                for (auto & geyser : geysers) {
                    if (geyser->Pos().getApproxDistance(hatch_center) < 400) {
                        double geyser_angle = Utility::FrogMath::get_angle(hatch_center, geyser->Pos());
                        angles.push_back(geyser_angle);
                    } 
                }
            }
            for (auto &mineral : minerals) {
                if (mineral->Pos().getApproxDistance(hatch_center) < 300) {
                    angles.push_back(Utility::FrogMath::get_angle(hatch_center, mineral->Pos()));
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
                        angles[j] += 2 * Utility::FrogMath::PI - sub_value;
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
                if (i != (unsigned)max_i && i != (unsigned)min_i) {
                    inside_i = i;
                    break;
                }
            }
            resource_blocking_angles[base_index][0] = saved_angles[min_i] - 0.4;
            if (resource_blocking_angles[base_index][0] < 0) {
                resource_blocking_angles[base_index][0] += 2 * Utility::FrogMath::PI;
            }
            resource_blocking_angles[base_index][1] = saved_angles[max_i] + 0.4;
            if (resource_blocking_angles[base_index][1] > 2 * Utility::FrogMath::PI) {
                resource_blocking_angles[base_index][1] -= 2 * Utility::FrogMath::PI;
            }
            resource_blocking_angles[base_index][2] = saved_angles[inside_i];
            
        }

        BNode find_node_at(int base_index, const BWAPI::TilePosition &tilepos) {
            if (tilepos.isValid()) {
                auto node_it = find_if(
                    _build_nodes[base_index].begin(), 
                    _build_nodes[base_index].end(), 
                    [tilepos]
                        (const BNode &bn)
                        {return bn->tilepos == tilepos;}
                );
                if (node_it != _build_nodes[base_index].end()) {
                    return *node_it;
                }
            }
            return nullptr;
        }

        void flag_resource_blocking_node(
            BNode node, 
            BWAPI::Position base_center,
            std::vector<double> &blocking_angles
        ) {
            double angle_to_node = Utility::FrogMath::get_angle(base_center, node->pos);
            if (Utility::FrogMath::angle_is_between(
                angle_to_node,
                blocking_angles[0],
                blocking_angles[1],
                blocking_angles[2]
            )) {
                node->blocks_mining = true;
            }
            else {
                node->blocks_mining = false;
            }
        }

        void seed_creep(int base_index, const BWAPI::TilePosition &tilepos) {
            const BWEM::Tile &_t = the_map.GetTile(tilepos);
            if (
                tilepos.isValid() 
                && find_node_at(base_index, tilepos) == nullptr
                && _t.Walkable()
                && Broodwar->hasCreep(tilepos)
            ) {
                BNode new_node = new BuildNode(_t, tilepos, node_ID_counter);
                flag_resource_blocking_node(
                    new_node, 
                    Bases::all_bases()[base_index]->Center(),
                    resource_blocking_angles[base_index]
                );
                _build_nodes[base_index].push_back(new_node);
                ++node_ID_counter;
            }
        }

        bool node_near_hatch(BNode node, std::vector<BWAPI::TilePosition> hatch_tilepositions) {
            BWAPI::TilePosition node_tilepos = node->tilepos;
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
                BNode check_node = _build_nodes[base_index][i];
                int disconnected_ct = 0;
                bool this_node_mineral_blocking = check_node->blocks_mining;
                
                for (auto &edge : check_node->edges) {
                    if (edge == nullptr) {
                        ++disconnected_ct;
                    }
                }
                if (disconnected_ct == 4 || !Broodwar->hasCreep(check_node->tilepos)) {
                    remove_queue.push_back(std::pair<BNode, int>(check_node, base_index));
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
                        const BWAPI::TilePosition tp = bnode->tilepos;
                        if (
                            !Broodwar->isBuildable(tp, true) 
                            ||
                            (!this_node_mineral_blocking && bnode->blocks_mining)
                            ||
                            bnode->blocks_larva
                            ||
                            bnode->reserved
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
                BNode cur_node = _build_nodes[base_index][i];
                const BWAPI::TilePosition &tp = cur_node->tilepos;
                if (Broodwar->isVisible(tp)) {
                    int cardinal[4][2] {{1, 0}, {0, -1}, {-1, 0}, {0, 1}};
                    for (int j = 0; j < 4; ++j) {
                        if (cur_node->edges[j] == nullptr) {
                            BWAPI::TilePosition check_tp(tp.x + cardinal[j][0], tp.y + cardinal[j][1]);
                            if (
                                check_tp.isValid()
                                && the_map.GetTile(check_tp).Walkable()
                                && Broodwar->hasCreep(check_tp)
                            ) {
                                BNode check_node = find_node_at(base_index, check_tp);
                                if (check_node == nullptr) {
                                    const BWEM::Tile &t = the_map.GetTile(check_tp);
                                    check_node = new BuildNode(t, check_tp, node_ID_counter);
                                    flag_resource_blocking_node(
                                        check_node, 
                                        Bases::all_bases()[base_index]->Center(),
                                        resource_blocking_angles[base_index]
                                    );
                                    _build_nodes[base_index].push_back(check_node);
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

        void remove_dead_nodes() {
            BNode edge;
            for (auto node : remove_queue) {
                auto &bnode = node.first;
                int base_index = node.second;
                if((edge = bnode->get_edge(RIGHT)) != nullptr)  edge->edges[LEFT] = nullptr;
                if((edge = bnode->get_edge(UP)) != nullptr)     edge->edges[DOWN] = nullptr;
                if((edge = bnode->get_edge(LEFT)) != nullptr)   edge->edges[RIGHT] = nullptr;
                if((edge = bnode->get_edge(DOWN)) != nullptr)   edge->edges[UP] = nullptr;
                auto node_it = std::remove(
                    _build_nodes[base_index].begin(), 
                    _build_nodes[base_index].end(), 
                    bnode
                );
                if (node_it != _build_nodes[base_index].end()) {
                    _build_nodes[base_index].erase(node_it);
                }
                delete bnode;
            }
            remove_queue.clear();
        }

        // checks if a build path is valid and returns the upper left (tilepos) BNode if so, nullptr if not
        BNode valid_build_path(int base_index, const BWAPI::TilePosition &tilepos, int width, int height) {
            BNode start_bnode = find_node_at(base_index, tilepos);
            BNode bnode = start_bnode;
            int 
                ref_side_steps = width - 1,
                side_steps = ref_side_steps,
                down_steps = height - 1;
            DIRECTIONS side_dir = RIGHT;
            bool bad_path = false;
            while (down_steps >= 0) {
                while (true) {
                    if (bnode == nullptr || bnode->occupied || bnode->reserved) {
                        bad_path = true;
                        break;
                    }
                    --side_steps;
                    if (side_steps < 0) {break;}
                    bnode = bnode->edges[side_dir];
                }
                if (bad_path) {break;}
                side_dir = (side_dir == RIGHT ? LEFT : RIGHT);
                side_steps = ref_side_steps;
                bnode = bnode->edges[DOWN];
                --down_steps;
            }
            return (bad_path ? nullptr : start_bnode);
        }

        // does not check path validity; should always be used after valid_build_path
        void set_build_path_reserved_status(BNode upper_left, int width, int height, bool status) {
            BNode bnode = upper_left;
            int 
                ref_side_steps = width - 1,
                side_steps = ref_side_steps,
                down_steps = height - 1;
            DIRECTIONS side_dir = RIGHT;
            while (down_steps >= 0) {
                while (true) {
                    bnode->reserved = status;
                    --side_steps;
                    if (side_steps < 0) {break;}
                    bnode = bnode->edges[side_dir];
                }
                side_dir = (side_dir == RIGHT ? LEFT : RIGHT);
                side_steps = ref_side_steps;
                bnode = bnode->edges[DOWN];
                --down_steps;
            }
        }
    }

    void init() {
        base_ct = Bases::all_bases().size();
        for (int i = 0; i < base_ct; ++i) {
            for (int j = 0; j < 3; ++j) {
                resource_blocking_angles[i].push_back(-1);
            }
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
            int base_nodes_sz = _build_nodes[i].size();
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
                try_expand(i);
                update_chunk(i);
                if (resource_blocking_angles[i][0] < 0) {
                    // TODO: change out with version that updates over the game,
                    // and change func so it doesn't group minerals and gas together,
                    // in case they're - for instance - on opposite sides
                    // Also, resource blocking nodes extend past resources,
                    // when they shouldn't
                    set_resource_blocking_angles(i);
                }
                start_chunk[i] += CHUNK_SIZE;
                end_chunk[i] += CHUNK_SIZE;
            }
        }
        remove_dead_nodes();
    }

    std::vector<BNode> *build_nodes() {
        return _build_nodes;
    }

    bool base_has_graph(const BWEM::Base *base) {
        auto &bases = Bases::all_bases();
        for (unsigned int i = 0; i < bases.size(); ++i) {
            auto &b = bases[i];
            if (b == base) {
                return _build_nodes[i].size() > 0;
            }
        }
        return false;
    }

    BWAPI::TilePosition get_build_tilepos(const BWEM::Base *base, int width, int height) {
        auto &bases = Bases::all_bases();
        for (unsigned int i = 0; i < bases.size(); ++i) {
            auto &b = bases[i];
            auto &base_nodes = _build_nodes[i];
            if (b == base && base_nodes.size() > 0) {
                for (auto &node : base_nodes) {
                    if (
                        node->buildable_dimensions[0] >= width
                        && node->buildable_dimensions[1] >= height
                        && !node->blocks_mining
                    ) {
                        return node->tilepos;
                    }
                }
            }
        }
        return BWAPI::TilePosition(-1, -1);
    }

    BWAPI::TilePosition get_geyser_tilepos(const BWEM::Base *base) {
        auto &geysers = base->Geysers();
        if (geysers.size() > 0) {
            return geysers[0]->TopLeft();
        }
        return BWAPI::TilePosition(-1, -1);
    }

    int make_reservation(
        const BWEM::Base *b,
        const BWAPI::TilePosition &tilepos, 
        int width, 
        int height
    ) {
        bool valid_reservation = false;
        auto &bases = Bases::all_bases();
        for (auto &base = bases.begin(); base != bases.end(); ++base) {
            if (*base == b) {
                BNode upper_left = valid_build_path(std::distance(bases.begin(), base), tilepos, width, height);
                if (upper_left != nullptr) {
                    set_build_path_reserved_status(upper_left, width, height, true);
                    return reservation_ID_counter++;
                }
                break;
            }
        }
        return -1;
    }

    void free_data() {
        for (int i = 0; i < base_ct; ++i) {
            const std::vector<BNode> &base_nodes = _build_nodes[i];
            for (unsigned int j = 0; j < base_nodes.size(); ++j) {
                delete base_nodes[j];
            }
            _build_nodes[i].clear();
        }
    }

// -----------------------------------------
// ------- getting base distances ----------
// -----------------------------------------

// for (int i = 0; i < base_ct; ++i) {
//     // for each base, create a sorted list of bases by walk-distance closeness
//     const BWEM::Base *from_base = all_bases[i];
//     bases[i] = from_base;
//     int distances[MAX_BASES] {-1};
//     for (int j = 0; j < base_ct; ++j) {
//         if (i != j) {
//             BWEB::Path path;  
//             path.createUnitPath(from_base->Center(), all_bases[j]->Center());                
//             distances[j] = path.getDistance();
//         }
//         else {
//             distances[j] = 0;
//         }
//     }
//     int prev_min = -1;
//     for (int j = 0; j < base_ct; ++j) {
//         int min_dist = INT_MAX;
//         int min_index = 0;
//         for (int k = 0; k < base_ct; ++k) {
//             if (distances[k] < min_dist && distances[k] > prev_min) {
//                 min_dist = distances[k];
//                 min_index = k;
//             }
//         }
//         closest[i][j] = all_bases[min_index];
//         prev_min = min_dist;
//     }
// }
}