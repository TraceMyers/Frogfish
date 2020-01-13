#pragma once

#include "../FrogFish.h"
#include "../unitdata/FrogBase.h"
#include <BWEM/bwem.h>
#include <BWAPI.h>

/*******************************************************************************
 * For regularly updating creep tiling at Zerg bases to aid building placement *
 ******************************************************************************/

////////////////////////////////////////+/////////////////////////////////////////
///                                 BuildNode                                   //
//////////////////////////////////////////////////////////////////////////////////
//       There is probably no need to interface directly with a BuildNode,
//       Except maybe to draw with them.
//       Create a BuildGraph, and the nodes will be generated.

class BuildNode {

private:

    const BWEM::Tile&        _tile;
    BWAPI::TilePosition      tilepos;
    std::vector<BuildNode *> edges;
    std::vector<int>         buildable_dimensions;
    bool                     blocks_mining;
    bool                     occupied;

public:

    int ID;

    BuildNode(const BWEM::Tile &_t, BWAPI::TilePosition tp, int _ID); 
    const BWEM::Tile &               bwem_tile();
    BWAPI::TilePosition              get_tilepos();
    const std::vector<BuildNode *> & get_edges();
    // Gives more or less up-to-date information about whether or not a zerg can build here
    // with a building size of at least 2x2 TilePositions (Creep Colony, etc.)
    // NOTE: there IS a delay between the truth and the update of this status.
    bool                             is_buildable();
    // Gives more or less up-to-date information about the size of building that can be
    // built here. 
    // NOTE: there IS a delay between the truth and the update of this status.
    const std::vector<int> &         get_buildable_dimensions();
    int                              get_ID();

/************************************Internal************************************/
    // Internal
    BuildNode *                      get_edge(int dir);
    // Internal
    void                             set_buildable_dimensions(int x, int y);
    // Internal
    void                             set_edge(int dir, BuildNode *n);
    // Internal
    bool                             operator ==(const BuildNode &other) const;

/************************************Internal************************************/
};

typedef BuildNode *                  BNode;


////////////////////////////////////////+/////////////////////////////////////////
///                                BuildGraph                                   //
//////////////////////////////////////////////////////////////////////////////////
// Create a BuildGraph to generate a creep build graph that automatically updates
// when creep spreads. Requires a pointer to an instance of type T1 to be 
// passed in, where T1 implements a member function named get_resource_depots(). 
// The function should return a const std::vector<T2 *> of hatcheries in the base. 
// T2 (the resource depot) needs a member function called get_tilepos() that 
// returns its TilePosition.

class BuildGraph {

private:

    FBase              base;
    std::vector<BNode> nodes;
    std::vector<BNode> geyser_nodes;
    int                node_ID_counter;
    int                start_chunk;
    int                end_chunk;
    int                CHUNK_SIZE;
    std::vector<BNode> remove_queue;

    void                       populate_graph(FUnit structure);
    void                       connect_nodes();
    void                       update_occupied_tilepositions();
    void                       update_chunk();
    void                       try_expand();
    BNode                      find_node_at(TilePosition &tilepos);
    void                       remove_dead_nodes();
    void                       seed_creep(FUnit structure);

public:

    enum               DIRECTIONS {RIGHT, UP, LEFT, DOWN};

    // A BuildGraph is tied to a Zerg base. The base can't be reassigned.
    BuildGraph();
    // Called when creep has already started growing at the base
    void                       init(FBase base);
    // Called in onFrame only after init()
    // Iterates over nodes, setting the size of building which can be placed at
    // each of them, if at all. Also expands the creep. Processes shouldn't
    // add any noticeable overhead, since they are updated in small chunks per
    // frame.
    void                       on_frame_update();
    //
    const std::vector<BNode> & get_nodes();
    // Checks whether the node at the TilePosition is buildable
    bool                       tilepos_buildable(TilePosition &tilepos);
    FBase                      get_base();
    // For keeping the Graph around when its Base is gone, and reusing the graph
    void                       clear();
    // Called at owner when no longer using this graph
    void                       free_data();
};