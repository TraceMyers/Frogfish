#pragma once
#pragma message("including BuildGraph")

#include "../FrogFish.h"
#include "../unitdata/FrogBase.h"
#include "../unitdata/BaseStorage.h"
#include "../unitdata/UnitStorage.h"
#include "../utility/BWTimer.h"
#include <BWEM/bwem.h>
#include <BWAPI.h>

////////////////////////////////////////+/////////////////////////////////////////
///                                BuildGraph                                   //
//////////////////////////////////////////////////////////////////////////////////
// Create a BuildGraph to generate a creep build graph that automatically updates
// when creep spreads. Requires a pointer to an instance of type T1 to be 
// passed in, where T1 implements a member function named get_resource_depots(). 
// The function should return a const std::vector<T2 *> of hatcheries in the base. 
// T2 (the resource depot) needs a member function called get_tilepos() that 
// returns its TilePosition.

namespace BuildGraph {
    void                       init(BaseStorage &base_storage);
    void                       on_frame_update(BaseStorage &base_storage);
    const std::vector<BNode> & get_nodes();
    bool                       tilepos_buildable(TilePosition &tilepos);
    void                       clear();
    void                       free_data();
};

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
    BWAPI::Position          pos;
    std::vector<BuildNode *> edges;
    std::vector<int>         buildable_dimensions;
    bool                     _blocks_mining;
    bool                     _blocks_larva;
    bool                     occupied;

public:

    int ID;

    BuildNode(const BWEM::Tile &_t, BWAPI::TilePosition tp, int _ID); 
    const BWEM::Tile &               bwem_tile();
    BWAPI::TilePosition              get_tilepos();
    BWAPI::Position                  get_pos(); 
    const std::vector<BuildNode *> & get_edges();
    // Gives more or less up-to-date information about whether or not a zerg can build here
    // with a building size of at least 2x2 TilePositions (Creep Colony, etc.)
    // NOTE: there IS a delay between the truth and the update of this status.
    bool                             is_buildable();
    // Gives more or less up-to-date information about the size of building that can be
    // built here. 
    // NOTE: there IS a delay between the truth and the update of this status.
    const std::vector<int> &         get_buildable_dimensions();
    bool                             blocks_mining();
    bool                             blocks_larva();
    int                              get_ID();

/************************************Internal************************************/
    // Internal
    void                             set_blocks_mining(bool value);
    // Internal
    void                             set_blocks_larva(bool value);
    // Internal
    BuildNode *                      get_edge(int dir);
    // Internal
    void                             set_buildable_dimensions(int x, int y);
    // Internal
    void                             set_edge(int dir, BuildNode *n);
    // Internal
    bool                             operator ==(const BuildNode &other) const;

};

typedef BuildNode *                  BNode;


