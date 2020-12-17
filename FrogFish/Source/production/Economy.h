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
    void            add_delay_to_build_order_sim(unsigned index, int delay_seconds, bool push_indices=false);
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
    bool            supply_blocked();
    unsigned int    make_reservation(int minerals, int gas, int reservation_seconds);
    bool            reservation_alive(unsigned ID);
    bool            end_reservation(unsigned ID);   
    bool            extend_reservation(unsigned ID, int seconds);
    void            print_sim_data(int limit=3);
    int             seconds_until_supply_blocked();
    int             build_order_ID_at_supply_block();
    const std::vector<std::pair<int, int>> &get_sim_data();
    void sim_set_just_added_overlord_flag_true();
}