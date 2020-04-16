#pragma once
#pragma message("including Construction")

#include "BuildOrder.h"
#include <BWAPI.h>

namespace Production::Construction {

    const int IN_BASE_TRAVEL_FRAMES = 80;

    void on_frame_update();
    void init_builds();

}