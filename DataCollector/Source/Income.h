#pragma once
#include <BWAPI.h>

namespace Income  {
    void init();
    void on_frame_update(const BWAPI::Player *players);
    bool ready();
    double *get_mps();
    double *get_gps();
}