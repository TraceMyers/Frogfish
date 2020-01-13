#pragma once

#include "BuildGraph.h"
#include "../unitdata/FrogBase.h"
#include "../unitdata/BaseStorage.h"
#include <vector>

namespace BuildPlacement {

    const int BASELEN = 20;

    void on_frame_update(BaseStorage &base_storage);

    void add_and_remove_bases(BaseStorage &base_storage);

    BuildGraph *get_graphs();
};
