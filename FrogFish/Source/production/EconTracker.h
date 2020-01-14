#pragma once
#pragma message("EconTracker")
#include "../utility/BWTimer.h"
#include <BWAPI.h>
#include <vector>

class EconTracker {

private:

    enum RESOURCES {MINERALS, GAS};

    enum FRAMESTUFF {FRAME_CT=5};

    BWAPI::Player self;
    BWTimer update_calc_timer;

    int
        calc_delay_seconds, 
        prev_total_minerals,
        prev_total_gas,
        prev_used_supply,
        reserved_minerals,
        reserved_gas,
        sf_head,
        sf_prev;
    unsigned int 
        reserved_resource_ID;
    double 
        supply_frames[FRAME_CT],
        minerals_per_sec,
        gas_per_sec,
        supply_per_sec;


    // if a resource is reserved without being used for too
    // long, the reservation times out and the resources are returned
    // to free_* 
    // reservations can also be cancelled, immediately returning the 
    // resources
    std::vector<unsigned int> reservation_IDs;
    std::vector<BWTimer *> reservation_timers;
    std::vector<int *> reserved_resources;

public:

    EconTracker(int _calc_delay_seconds=8); 
    void init();

    void on_frame_update();
    // TODO: this probably needs to get more accurate in the future
    // 15 seconds is a long delay to get new economic information
    // maybe use 15 second frames, and just move the frame forward
    void calc_stats();
    // creates a curved average distribution of recent supply
    // for more accurate averaging
    double supply_frames_average();
    double get_minerals_per_sec(); 
    double get_gas_per_sec();
    double get_supply_per_sec();
    int get_free_minerals();
    int get_free_gas();
    // returns reference ID
    // allows current resources - reserved resources to go negative
    unsigned int make_reservation(int minerals, int gas, int reservation_seconds);
    bool reservation_alive(unsigned int ID);
    // used internally and externally. Works for killing/canceling a res,
    // or for noting that the res is filled
    bool end_reservation(unsigned int ID);   
    bool extend_reservation(unsigned int ID, int seconds);
};