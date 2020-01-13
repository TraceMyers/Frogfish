#include "BuildGraph.h"
// #include "../unitdata/FrogBase.h"
#include "../unitdata/FrogUnit.h"
#include <BWAPI.h>
#include <BWEM/bwem.h>

class FrogBase;
typedef FrogBase *FBase;

////////////////////////////////////////+/////////////////////////////////////////
///                                 BuildNode                                   //
//////////////////////////////////////////////////////////////////////////////////

BuildNode::BuildNode(const BWEM::Tile &_t, BWAPI::TilePosition tp, int _ID) : 
    _tile(_t), 
    tilepos(tp),
    buildable_dimensions({0, 0}),
    edges({nullptr, nullptr, nullptr, nullptr}),
    ID(_ID),
    occupied(false)
{}

const BWEM::Tile &BuildNode::bwem_tile() {return _tile;}

BWAPI::TilePosition BuildNode::get_tilepos() {return tilepos;}

void BuildNode::set_edge(int dir, BuildNode *n) {edges[dir] = n;}

BuildNode *BuildNode::get_edge(int dir) {return edges[dir];}

const std::vector<BuildNode *> &BuildNode::get_edges() {return edges;}

bool BuildNode::is_buildable() {return buildable_dimensions[0] >= 1;}

void BuildNode::set_buildable_dimensions(int x, int y) {
    buildable_dimensions[0] = x;
    buildable_dimensions[1] = y;
}

const std::vector<int> &BuildNode::get_buildable_dimensions() {return buildable_dimensions;}

int BuildNode::get_ID() {return ID;}

bool BuildNode::operator ==(const BuildNode &other) const {return this->ID == other.ID;}

                                         
////////////////////////////////////////+/////////////////////////////////////////
///                                BuildGraph                                   //
//////////////////////////////////////////////////////////////////////////////////

// TODO: template <class BaseT>
// TODO: Integrate with BuildPlacement, have a good system in place
// to simplify placement decisionmaking. 
BuildGraph::BuildGraph() : base(nullptr) {}

void BuildGraph::init(FBase _base) {
    node_ID_counter = 0;
    start_chunk = 0;
    end_chunk = 10000;
    CHUNK_SIZE = 3;
    base = _base;
    for (auto & hatch : base->get_resource_depots()) {
        // populate_graph(hatch);
        seed_creep(hatch);
    }
    // connect_nodes();
}

FBase BuildGraph::get_base() {
    return base;
}

// TODO: check base for new hatcheries & creep colonies, set creep seed
void BuildGraph::on_frame_update() {
    int nodes_size = nodes.size();
    if (nodes_size > 0) {
        if (end_chunk > nodes_size) {
            if (CHUNK_SIZE >= nodes_size) {
                end_chunk = nodes_size;
            }
            else {
                end_chunk = CHUNK_SIZE;
            }
            start_chunk = 0;
        }
        update_chunk();
        try_expand();
        remove_dead_nodes();
        start_chunk += CHUNK_SIZE;
        end_chunk += CHUNK_SIZE;
    }
    for (auto & structure : base->get_structures()) {
        seed_creep(structure);
    }
}

void BuildGraph::seed_creep(FUnit structure) {
    TilePosition structure_tilepos = structure->get_tilepos();
    const BWEM::Tile &_t = the_map.GetTile(structure_tilepos);
    if (
        Broodwar->hasCreep(structure_tilepos)
        && structure_tilepos.isValid() 
        && find_node_at(structure_tilepos) == nullptr
    ) {
        nodes.push_back(new BuildNode(_t, structure_tilepos, node_ID_counter));
        ++node_ID_counter;
    }
}

void BuildGraph::populate_graph(FUnit structure) {
    TilePosition hatch_tilepos = structure->get_tilepos();
    int 
        corner_step = 2,
        next_square_diff[5][2] {{0, 0}, {1, 0}, {0, -1}, {-1, 0}, {0, 1}},
        outer_step = 1,
        steps_this_turn = 8,
        i,
        inner_step,
        creep_ct;
    const BWEM::Tile &hatch_t = the_map.GetTile(hatch_tilepos);
    nodes.push_back(new BuildNode(hatch_t, hatch_tilepos, node_ID_counter));
    while(true) {
        i = 0;
        inner_step = 0;
        creep_ct = 0;
        BWAPI::TilePosition check_square(
            hatch_tilepos.x - outer_step, hatch_tilepos.y - outer_step
        );
        while (inner_step < steps_this_turn) {
            check_square = BWAPI::TilePosition(
                check_square.x + next_square_diff[i][0], 
                check_square.y + next_square_diff[i][1]
            );
            if (
                check_square.isValid()
                && Broodwar->hasCreep(check_square)
            ) {
                const BWEM::Tile &t = the_map.GetTile(check_square);
                nodes.push_back(new BuildNode(t, check_square, node_ID_counter));
                ++creep_ct;
                ++node_ID_counter;
            }
            if (inner_step % corner_step == 0) {
                ++i;
            }
            ++inner_step;
        }
        if (creep_ct == 0) {
            break;
        }
        corner_step += 2;
        ++outer_step;
        steps_this_turn += 8;
    }
}

void BuildGraph::connect_nodes() {
    for (auto & node : nodes) {
        TilePosition node_pos = node->get_tilepos();
        int directions[4][2] {{1, 0}, {0, -1}, {-1, 0}, {0, 1}};
        for (int i = 0; i < 4; ++i) {
            TilePosition neighbor_tilepos(
                node_pos.x + directions[i][0], node_pos.y + directions[i][1]
            );
            std::vector<BNode>::iterator neighbor_node = find_if(
                nodes.begin(), 
                nodes.end(), 
                [neighbor_tilepos]
                    (const BNode &bn)
                    {return bn->get_tilepos() == neighbor_tilepos;}
            );
            if (neighbor_node != nodes.end()) {
                node->set_edge(i, *neighbor_node);
            }
        }
    }
}

void BuildGraph::update_chunk() {
    int path[11] {LEFT, DOWN, RIGHT, RIGHT, UP, RIGHT, DOWN, DOWN, LEFT, LEFT, LEFT};
    for (int check_i = start_chunk; check_i < end_chunk; ++check_i) {
        register bool buildable[12] {false};
        BNode check_node = nodes[check_i];
        int disconnected_ct = 0;
        for (auto &edge : check_node->get_edges()) {
            if (edge == nullptr) {
                ++disconnected_ct;
            }
        }
        if (disconnected_ct == 4 || !Broodwar->hasCreep(check_node->get_tilepos())) {
            remove_queue.push_back(check_node);
        }
        else {
            BNode bnode = check_node->get_edge(0);
            int i;
            if (bnode != nullptr) {
                for (i = 0; i < 12; ++i) {
                    const TilePosition &tp = bnode->get_tilepos();
                    if (Broodwar->isBuildable(tp, true)) {
                        buildable[i] = true;
                    }
                    if (i == 3) {
                        if (buildable[0] && buildable[1] && buildable[2] && buildable[3]) {
                            check_node->set_buildable_dimensions(2, 2);
                        }
                        else {
                            check_node->set_buildable_dimensions(0, 0);
                            break;
                        }
                    }
                    else if (i == 5) {
                        if (buildable[4] && buildable[5]) {
                            check_node->set_buildable_dimensions(3, 2);
                        }
                        else {
                            break;
                        }
                    }
                    else if (i == 7) {
                        if (buildable[6] && buildable[7]) {
                            check_node->set_buildable_dimensions(4, 2);
                        }
                        else {
                            break;
                        }
                    }
                    else if (i == 11) {
                        if (buildable[8] && buildable[9] && buildable[10] && buildable[11]) {
                            check_node->set_buildable_dimensions(4, 3);
                        }
                        break;
                    }
                    bnode = bnode->get_edge(path[i]);
                    if (bnode == nullptr) {
                        break;
                    }
                }
            }
        }
    }
}

void BuildGraph::try_expand() {
    for (int check_i = start_chunk; check_i < end_chunk; ++check_i) {
        BNode cur_node = nodes[check_i];
        BWAPI::TilePosition tp = cur_node->get_tilepos();
        if (Broodwar->isVisible(tp)) {
            int node_path[8] {LEFT, UP, RIGHT, RIGHT, DOWN, DOWN, LEFT, LEFT};
            int step_path[8][2] {{-1, 0}, {0, -1}, {1, 0}, {1, 0}, {0, 1}, {0, 1}, {-1, 0}, {-1, 0}};
            BNode new_nodes[8] {nullptr};
            BWAPI::TilePosition cur_tp = tp;
            for (int i = 0; i < 8 ; ++i) {
                bool candidate_tp = true;
                cur_tp = BWAPI::TilePosition(cur_tp.x + step_path[i][0], cur_tp.y + step_path[i][1]);
                if (cur_node != nullptr) {
                    cur_node = cur_node->get_edge(node_path[i]);
                    if (cur_node != nullptr) {
                        candidate_tp = false;
                    }
                }
                if (
                    candidate_tp
                    && cur_tp.isValid() 
                    && Broodwar->hasCreep(cur_tp)
                ) {
                    BNode bn = find_node_at(cur_tp);
                    if (bn == nullptr) {
                        const BWEM::Tile &t = the_map.GetTile(cur_tp);
                        BNode new_node = new BuildNode(t, cur_tp, node_ID_counter);
                        new_nodes[i] = new_node;
                        nodes.push_back(new_node);
                        ++node_ID_counter;
                    }
                }
            }
            for (int i = 0; i < 8; ++i) {
                BNode new_node = new_nodes[i];
                if (new_node != nullptr) {
                    TilePosition tp_new = new_node->get_tilepos();
                    int step_directions[4][2] = {{1, 0}, {0, -1}, {-1, 0}, {0, 1}};
                    for (int j = 0; j < 4; ++j) {
                        BWAPI::TilePosition potential_edge(
                            tp_new.x + step_directions[j][0],
                            tp_new.y + step_directions[j][1]
                        );
                        BNode check_if_node = find_node_at(potential_edge);
                        if (check_if_node != nullptr) {
                            new_node->set_edge(j, check_if_node);
                            if (j == 2 || j == 3) {
                                check_if_node->set_edge(j - 2, new_node);
                            }
                            else {
                                check_if_node->set_edge(j + 2, new_node);
                            }
                        }
                    }
                }
            }
        }
    }
}

const std::vector<BNode> &BuildGraph::get_nodes() {return nodes;}

bool BuildGraph::tilepos_buildable(TilePosition &tilepos) {
    if (tilepos.isValid()) {
        auto node = find_if(
            nodes.begin(), 
            nodes.end(), 
            [tilepos]
                (const BNode &bn)
                {return bn->get_tilepos() == tilepos;}
        );
        if (node != nodes.end()) {
            return (*node)->is_buildable(); 
        }
    }
    return false;
}

BNode BuildGraph::find_node_at(TilePosition &tilepos) {
    if (tilepos.isValid()) {
        auto node = find_if(
            nodes.begin(), 
            nodes.end(), 
            [tilepos]
                (const BNode &bn)
                {return bn->get_tilepos() == tilepos;}
        );
        if (node != nodes.end()) {
            return *node;
        }
    }
    return nullptr;
}

void BuildGraph::remove_dead_nodes() {
    BNode edge;
    for (auto node : remove_queue) {
        if((edge = node->get_edge(RIGHT)) != nullptr) edge->set_edge(LEFT, nullptr);
        if((edge = node->get_edge(UP)) != nullptr) edge->set_edge(DOWN, nullptr);
        if((edge = node->get_edge(LEFT)) != nullptr) edge ->set_edge(RIGHT, nullptr);
        if((edge = node->get_edge(DOWN)) != nullptr) edge ->set_edge(UP, nullptr);
        auto node_it = std::find(nodes.begin(), nodes.end(), node);
        if (node_it != nodes.end()) {
            nodes.erase(node_it);
        }
        delete node;
    }
    remove_queue.clear();
}

void BuildGraph::clear() {
    free_data();
    nodes.clear();
    base = nullptr;
}

// called @ owner base when owner base destroyed
void BuildGraph::free_data() {
    for (auto &node : nodes) {
        delete node;
    }
}