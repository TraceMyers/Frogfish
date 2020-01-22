#pragma once
#pragma message("including BuildPlacement")

#include "BuildGraph.h"
#include "../unitdata/FrogBase.h"
#include "../unitdata/BaseStorage.h"
#include "../unitdata/UnitStorage.h"
#include <BWEM/bwem.h>
#include <vector>

namespace BuildPlacement {

    const int MAX_BASES = 20;

    void init(BaseStorage &base_storage);

    void on_frame_update(BaseStorage &base_storage);

    void add_and_remove_bases(BaseStorage &base_storage);

    TilePosition find_any_node_for_placement(FBase base, int width, int height);

    TilePosition find_node_for_tech_placement(FBase base, int width, int height);

    TilePosition find_expansion_tilepos(BaseStorage &base_storage, FBase base);

    TilePosition get_base_geyser_tilepos(FBase base);

    TilePosition get_pathable_geyser_tilepos(TilePosition geyser_tp);

    BuildGraph *get_graphs();

    bool graph_ready(FBase base);
}
