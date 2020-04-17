#pragma once
#pragma message("including Construction")

#include "BuildOrder.h"
#include <BWAPI.h>

namespace Production::Construction {
    void init();
    void on_frame_update();
    bool worker_reserved_for_building(const BWAPI::Unit unit);
}