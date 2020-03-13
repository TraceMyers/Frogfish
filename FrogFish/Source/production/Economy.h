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
    void            init();
    void            on_frame_update();
    double          get_minerals_per_frame(); 
    double          get_gas_per_frame();
    double          get_larva_per_frame();
    double          get_supply_per_frame();
    double          get_minerals_per_sec(); 
    double          get_gas_per_sec();
    double          get_larva_per_sec();
    double          get_supply_per_sec();
    int             get_free_minerals();
    int             get_free_gas();
    unsigned int    make_reservation(int minerals, int gas, int reservation_seconds);
    bool            reservation_alive(unsigned ID);
    bool            end_reservation(unsigned ID);   
    bool            extend_reservation(unsigned ID, int seconds);
    void            print_sim_data();
    int             seconds_until_supply_blocked();
    const std::vector<std::vector<int>> &sim_data();
}