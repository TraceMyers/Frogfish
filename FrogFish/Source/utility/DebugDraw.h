#pragma once
#pragma message("including DebugDraw")

#include "../FrogFish.h"
#include <BWEM/bwem.h>
#include <string>

namespace Utility::DebugDraw {
    void append_debug_text(std::string *dbg_line);
    void draw_debug_text();
    void draw_units();
    // void draw_map();
    // void draw_base_info(BaseStorage &base_storage);
    // void draw_make_queue(MakeQueue &make_queue);
    // void draw_build_graphs();
    // void draw_bwem_data();
}