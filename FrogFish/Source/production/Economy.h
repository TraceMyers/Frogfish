#pragma once
#pragma message("including Economy")

#include "BuildOrder.h"
#include "../utility/BWTimer.h"
#include "../basic/Bases.h"
#include "../basic/Units.h"
#include <BWAPI.h>
#include <BWEM/bwem.h>
#include <vector>
#include <fstream>

namespace Production::Economy {
    void init();
    void on_frame_update();

    double get_minerals_per_frame(); 
    double get_gas_per_frame();
    double get_larva_per_frame();
    double get_supply_per_frame();
    double get_minerals_per_sec(); 
    double get_gas_per_sec();
    double get_larva_per_sec();
    double get_supply_per_sec();

    int get_free_minerals();
    int get_free_gas();
    // returns reference ID
    // allows (current resources - reserved resources) to go negative
    unsigned int make_reservation(int minerals, int gas, int reservation_seconds);
    bool reservation_alive(unsigned int ID);
    // used internally and externally. Works for killing/canceling a res,
    // or for noting that the res is filled
    bool end_reservation(unsigned int ID);   
    bool extend_reservation(unsigned int ID, int seconds);

    // Iterates over build order and returns how many seconds
    // until each count of each item can be built.
    // Takes a build order, and an index/ID at which to start.
    // making_types_in is all of the types currently making
    // making_frames_left_in is how many frames those units have left to finish making
    std::vector<std::vector<int>> simulate(int seconds=360);
}