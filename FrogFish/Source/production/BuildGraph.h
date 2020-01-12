#pragma once

#include <BWEM/bwem.h>
#include <BWAPI.h>
#include "../data/FrogBase.h"

// TODO: set blocks_mining

class BuildNode {

private:

    const BWEM::Tile &tile;
    BWAPI::TilePosition tilepos;
    std::vector<BuildNode *> edges;
    std::vector<int> buildable_dimensions;
    bool immediately_buildable;
    bool blocks_mining;
    bool occupied;

public:

    int ID;

    BuildNode(const BWEM::Tile &_t, BWAPI::TilePosition tp, int _ID) : 
        tile(_t), 
        tilepos(tp),
        buildable_dimensions({0, 0}),
        edges({nullptr, nullptr, nullptr, nullptr}),
        immediately_buildable(false),
        ID(_ID),
        occupied(false)
    {}

    // BuildNode(const BWEM::Tile &_t, BWAPI::TilePosition tp);

    const BWAPI::TilePosition &get_tilepos();

    void set_edge(int dir, BuildNode *n);

    BuildNode *get_edge(int dir);

    const std::vector<BuildNode *> &get_edges();

    void set_buildable_dimensions(int x, int y);

    // buildable_dimensions tells you what size building can be made ON this Node
    // (takes into account neighboring tiles, buildings, statics & creep)
    const std::vector<int> &get_buildable_dimensions();

    // Depending on how often the graph is updated, gives more or less up-to-date information
    // about whether or not this tile and its required neighboring tiles have creep, and
    // are not blocked by buildings
    bool is_buildable();

    void set_immediately_buildable(bool value);

    // Depending on how often the graph is updated, gives more or less up-to-date information
    // about whether or not this tile is blocked by a unit (and is buildable)
    // might change to be as-certain-as-possible this frame
    bool is_immediately_buildable();

    int get_ID();

    bool BuildNode::operator ==(const BuildNode &other) const;
};

typedef BuildNode *BNode;

// template <class BaseT>
class BuildGraph {

private:

    FBase base = nullptr;
    std::vector<BNode> nodes;
    std::vector<std::vector<BNode>> node_chunks;
    std::vector<BNode> geyser_nodes;
    int node_ID_counter;
    int chunk_counter;

public:

    enum DIRECTIONS {RIGHT, UP, LEFT, DOWN};

    // BuildGraph(BWEM::Map &_the_map, FBase _base);
    BuildGraph();

    void init(BWEM::Map &_the_map, FBase _base);

    // Loops over each resource depot and radially checks for creep
    // from their centers, adding new nodes to the map.
    void populate_graph(BWEM::Map &the_map, FBase _base);

    void connect_nodes();

    void on_frame_update();

    void update_occupied_tilepositions();

    // Chunks are box-rings of buildable tiles surrounding a hatchery.
    // 1. Checks one chunk to make sure creep is still there, and no buildings are in the way
    // 2. Updates the buildable size of each node, which is the size of the building
    // that can be placed AT this node (checked by referencing neighbor nodes)
    // 3. TODO: set whether or not the tile is immediately buildable: whether
    // or not a unit is in the way.
    void update_chunk();

    void try_expand();

    const std::vector<BNode> &get_nodes();

    bool tilepos_buildable(TilePosition &tilepos);
 
    // called @ owner during onEnd()
    void free_data();
};