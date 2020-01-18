#pragma once
#pragma message("including BuildPlacement")

#include "BuildGraph.h"
#include "../unitdata/FrogBase.h"
#include "../unitdata/BaseStorage.h"
#include <BWEM/bwem.h>
#include <vector>

namespace BuildPlacement {

    const int BASELEN = 20;

    void on_frame_update(BaseStorage &base_storage);

    void add_and_remove_bases(BaseStorage &base_storage);

    TilePosition find_any_node_for_placement(FBase base, int width, int height);

    TilePosition find_node_for_tech_placement(FBase base, int width, int height);

    TilePosition get_base_geyser_tilepos(FBase base);

    BuildGraph *get_graphs();
}
