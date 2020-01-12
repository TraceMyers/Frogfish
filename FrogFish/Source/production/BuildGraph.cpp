#include "../data/FrogBase.h"
#include "BuildGraph.h"
#include <BWAPI.h>
#include <BWEM/bwem.h>


    // Only set by BuildGraph::Graph
    // Not sure if you can rely on the tile ref being good.



const BWAPI::TilePosition &BuildNode::get_tilepos() {return tilepos;}

void BuildNode::set_edge(int dir, BuildNode *n) {
    edges[dir] = n;
}

void BuildNode::remove_edge(int dir) {
    edges[dir] = nullptr;
}

BuildNode *BuildNode::get_edge(int dir) {return edges[dir];}

const std::vector<BuildNode *> &BuildNode::get_edges() {return edges;}

void BuildNode::set_buildable_dimensions(int x, int y) {
    buildable_dimensions[0] = x;
    buildable_dimensions[1] = y;
}

// buildable_dimensions tells you what size building can be made ON this Node
// (takes into account neighboring tiles, buildings, statics & creep)
const std::vector<int> &BuildNode::get_buildable_dimensions() {return buildable_dimensions;}

// Depending on how often the graph is updated, gives more or less up-to-date information
// about whether or not this tile and its required neighboring tiles have creep, and
// are not blocked by buildings
bool BuildNode::is_buildable() {return buildable_dimensions[0] >= 1;}

void BuildNode::set_immediately_buildable(bool value) {immediately_buildable = value;}

// Depending on how often the graph is updated, gives more or less up-to-date information
// about whether or not this tile is blocked by a unit (and is buildable)
// might change to be as-certain-as-possible this frame
bool BuildNode::is_immediately_buildable() {return immediately_buildable;}

int BuildNode::get_ID() {return ID;}

bool BuildNode::operator ==(const BuildNode &other) const {
    return this->ID == other.ID;
}


// template <class BaseT>


/* BuildGraph(BWEM::Map &_the_map, FBase _base) : 
    the_map(_the_map), 
    base(_base),
    nodes(100),
    node_chunks(6),
    geyser_nodes(2)
{
    populate_graph(_the_map, _base);
    connect_nodes();
}
*/

BuildGraph::BuildGraph() : node_ID_counter(0), chunk_counter(0) {}

// temp
void BuildGraph::init(BWEM::Map &_the_map, FBase _base) {
    // the_map = _the_map;
    base = _base;
    // nodes.resize(100);
    // node_chunks.resize(6);
    populate_graph(_the_map, base);
    connect_nodes();
    expand_counter = 0;
}

// Loops over each resource depot and radially checks for creep
// from their centers, adding new nodes to the map.
void BuildGraph::populate_graph(BWEM::Map &the_map, FBase _base) {
    for (auto & hatch : _base->get_resource_depots()) {
        TilePosition hatch_tilepos = hatch->get_tilepos();
        int 
            corner_step = 2,
            next_square_diff[5][2] {{0, 0}, {1, 0}, {0, 1}, {-1, 0}, {0, -1}},
            outer_step = 1,
            steps_this_turn = 8,
            i,
            inner_step,
            build_ct,
            chunk = 0;
        node_chunks.push_back(std::vector<BNode>());
        const BWEM::Tile &t = the_map.GetTile(hatch_tilepos);
        nodes.push_back(new BuildNode(t, hatch_tilepos, node_ID_counter));
        node_chunks[chunk].push_back(nodes.back());
        while(true) {
            i = 0;
            inner_step = 0;
            build_ct = 0;
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
                    node_chunks[chunk].push_back(nodes.back());
                    ++build_ct;
                    ++node_ID_counter;
                }
                if (inner_step % corner_step == 0) {
                    ++i;
                }
                ++inner_step;
            }
            if (build_ct < steps_this_turn) {
                break;
            }
            node_chunks.push_back(std::vector<BNode>());
            corner_step += 2;
            ++outer_step;
            steps_this_turn += 8;
            ++chunk;
        }
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
            auto neighbor_node = find_if(
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

void BuildGraph::on_frame_update(BWEM::Map &the_map) {
    remove_dead_chunks();
    if (node_chunks.size() > 0) {
        try_expand(the_map);
        update_chunk();
        remove_dead_nodes();
    }
}

void BuildGraph::remove_dead_chunks() {
    int sz;
    while ((sz = node_chunks.size()) > 0 && node_chunks[sz - 1].size() == 0) {
        node_chunks.erase(node_chunks.end() - 1);
        expand_counter = 0;
    }
}

void BuildGraph::update_chunk() {
    int path[11] {LEFT, DOWN, RIGHT, RIGHT, UP, RIGHT, DOWN, DOWN, LEFT, LEFT, LEFT};
    for (auto & bnode : node_chunks[chunk_counter]) {
        register bool buildable[12] {false};
        BNode check_node = bnode;
        const TilePosition &tp = check_node->get_tilepos();
        if (!Broodwar->isVisible(tp)) {
            continue;
        }
        if (!Broodwar->hasCreep(check_node->get_tilepos())) {
            remove_queue.push_back(check_node);
        }
        else {
            check_node = bnode->get_edge(0);
            int i;
            if (check_node != nullptr) {
                for (i = 0; i < 12; ++i) {
                    const TilePosition &tp = check_node->get_tilepos();
                    if (Broodwar->isBuildable(tp, true)) {
                        buildable[i] = true;
                    }
                    if (i == 3) {
                        if (buildable[0] && buildable[1] && buildable[2] && buildable[3]) {
                            bnode->set_buildable_dimensions(2, 2);
                        }
                        else {
                            bnode->set_buildable_dimensions(0, 0);
                            break;
                        }
                    }
                    else if (i == 5) {
                        if (buildable[4] && buildable[5]) {
                            bnode->set_buildable_dimensions(3, 2);
                        }
                        else {
                            break;
                        }
                    }
                    else if (i == 7) {
                        if (buildable[6] && buildable[7]) {
                            bnode->set_buildable_dimensions(4, 2);
                        }
                        else {
                            break;
                        }
                    }
                    else if (i == 11) {
                        if (buildable[8] && buildable[9] && buildable[10] && buildable[11]) {
                            bnode->set_buildable_dimensions(4, 3);
                        }
                        break;
                    }
                    check_node = check_node->get_edge(path[i]);
                    if (check_node == nullptr) {
                        break;
                    }
                }
            }
        }
    }
    ++chunk_counter;
    if (chunk_counter == node_chunks.size()) {
        chunk_counter = 0;
    }
}

void BuildGraph::try_expand(BWEM::Map &the_map) {
    int node_chunks_size = node_chunks.size();
    auto &outer_chunk = node_chunks[node_chunks_size - 1];
    BNode cur_node = outer_chunk[expand_counter];
    BWAPI::TilePosition tp = cur_node->get_tilepos();
    if (Broodwar->isVisible(tp)) {
        int node_path[8] {LEFT, UP, RIGHT, RIGHT, DOWN, DOWN, LEFT, LEFT};
        int step_path[8][2] {{-1, 0}, {0, -1}, {1, 0}, {1, 0}, {0, 1}, {0, 1}, {-1, 0}, {-1, 0}};
        BNode new_nodes[8] {nullptr};
        BNode next;
        BWAPI::TilePosition next_tp = tp;
        for (int i = 0; i < 8 ; ++i) {
            next_tp = BWAPI::TilePosition(next_tp.x + step_path[i][0], next_tp.y + step_path[i][1]);
            bool tp_is_candidate = false;
            if (cur_node != nullptr) {
                // printf("ID: %d\n", cur_node->get_ID());
                next = cur_node->get_edge(node_path[i]);
                if (cur_node == nullptr) {
                    tp_is_candidate = true;
                }
            }
            if (
                (tp_is_candidate || (cur_node = find_node_at(next_tp)) == nullptr)
                && next_tp.isValid() 
                && Broodwar->hasCreep(next_tp)
            ) {
                printf("new tile\n");
                const BWEM::Tile &t = the_map.GetTile(next_tp);
                BNode new_node = new BuildNode(t, next_tp, node_ID_counter);
                new_nodes[i] = new_node;
                nodes.push_back(new_node);
                outer_chunk.push_back(new_node);
                ++node_ID_counter;
            }
        }
        for (int i = 0; i < 8; ++i) {
            BNode new_node = new_nodes[i];
            if (new_node != nullptr) {
                const TilePosition &tp = new_node->get_tilepos();
                int step_directions[4][2] = {{1, 0}, {0, -1}, {-1, 0}, {0, 1}};
                for (int i = 0; i < 4; ++i) {
                    BNode check_if_node = find_node_at(
                        BWAPI::TilePosition(
                            tp.x + step_directions[i][0],
                            tp.y + step_directions[i][1]
                        )
                    );
                    if (check_if_node != nullptr) {
                        new_node->set_edge(i, check_if_node);
                        if (i == 2 || i == 3) {
                            check_if_node->set_edge(i - 2, new_node);
                        }
                        else {
                            check_if_node->set_edge(i + 2, new_node);
                        }
                    }
                }
            }
        }
    }
    int outer_chunk_size = outer_chunk.size();
    ++expand_counter;
    if (expand_counter >= outer_chunk_size) {
        expand_counter = 0;
    }
    if (outer_chunk_size >= 20) {
        // node_chunks.push_back(std::vector<BNode>());
        // node_chunks[node_chunks_size].push_back(outer_chunk[outer_chunk_size - 1]);
        // outer_chunk.erase(outer_chunk.end() - 1);
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
    for (auto &node : remove_queue) {
        if((edge = node->get_edge(RIGHT)) != nullptr) edge->remove_edge(LEFT);
        if((edge = node->get_edge(UP)) != nullptr) edge->remove_edge(DOWN);
        if((edge = node->get_edge(LEFT)) != nullptr) edge ->remove_edge(RIGHT);
        if((edge = node->get_edge(DOWN)) != nullptr) edge ->remove_edge(UP);
        auto node_it = find(nodes.begin(), nodes.end(), node);
        if (node_it != nodes.end()) {
            nodes.erase(node_it);
        }
        for (auto &vec : node_chunks) {
            auto node_it = find(vec.begin(), vec.end(), node);
            if (node_it != vec.end()) {
                vec.erase(node_it);
                break;
            }
        }
        delete node;
    }
    remove_queue.clear();
}

// called @ owner base when owner base destroyed
void BuildGraph::free_data() {
    for (auto &node : nodes) {
        delete node;
    }
}