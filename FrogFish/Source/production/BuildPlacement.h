#pragma once
#pragma message("BuildPlacement")

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

    const std::vector<BWEM::Geyser *> &get_base_geysers(FBase base);

    BuildGraph *get_graphs();
}
