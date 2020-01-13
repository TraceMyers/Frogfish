#pragma once

#include "../FrogFish.h"
#include "../unitdata/UnitStorage.h"
#include "../unitdata/BaseStorage.h"
#include "../production/MakeQueue.h"
#include "../production/BuildManager.h"
#include <BWEM/bwem.h>
#include <string>

namespace DebugDraw {

    void append_debug_text(std::string *dbg_line);
    void draw_debug_text();
    void draw_units(UnitStorage &us);
    void draw_map();
    void draw_base_info(BaseStorage &base_storage);
    void draw_make_queue(MakeQueue &make_queue);
    void draw_build_graphs();
}