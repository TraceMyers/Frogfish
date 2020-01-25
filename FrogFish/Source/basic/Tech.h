#pragma once
#pragma message("including Tech")

#include "Bases.h"
#include <BWAPI.h>

// TODO: implement enemy as well

namespace Basic::Tech {
    void on_frame_update();
    bool self_can_make(BWAPI::UnitType &ut);
}