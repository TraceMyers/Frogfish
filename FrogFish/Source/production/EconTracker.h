#pragma once
#pragma message("including EconTracker")

#include "../utility/BWTimer.h"
#include "../unitdata/FrogUnit.h"
#include "../unitdata/FrogBase.h"
#include "../unitdata/BaseStorage.h"
#include "../unitdata/UnitStorage.h"
#include "BuildOrder.h"
#include <BWAPI.h>
#include <BWEM/bwem.h>
#include <vector>
#include <fstream>

class EconTracker {

private:

    enum RESOURCES {MINERALS, GAS};

    BWAPI::Player self;
    BWTimer update_calc_timer;

    const int
        SUPPLY_FRAME_SECONDS = 8,
        SUPPLY_FRAME_CT = 3,
        FINISH_TIME = 48;
    int
        reserved_minerals,
        reserved_gas,
        prev_supply,
        supply_frames[3],
        larva_ct;
    unsigned int 
        reserved_resource_ID;
	double
		supply_per_frame,
		minerals_per_frame,
		gas_per_frame,
		larva_per_frame;
    // constants either gotten from regression or simple averaging.
    // MPF_SIMPLE_CONST is the added minerals per frame per drone for build order simming,
    // and it assumes a drone-to-mineral ratio of 1
	const double
		MPF_INTERCEPT = 0.0536346423597,
		MPF_SATURATION_COEF = 0.02770819,
		MPF_SATURATION_SQ_COEF = -0.01181919,
		MPF_SATURATION_CU_COEF = 0.00121287,
        MPF_SIMPLE_CONST = MPF_INTERCEPT + MPF_SATURATION_COEF 
            + 4.0 * MPF_SATURATION_SQ_COEF + 8.0 * MPF_SATURATION_CU_COEF,
		GPF_CONST = 0.071296296,
		LPF_CONST = 0.002777778,
        SUPPLY_FRAME_POLY_FACT = 0.54369;
    BWTimer 
        supply_frame_timer;

    // if a resource is reserved without being used for too
    // long, the reservation times out and the resources are returned
    // to free_* 
    // reservations can also be cancelled, immediately returning the 
    // resources
    std::vector<unsigned int> reservation_IDs;
    std::vector<BWTimer *> reservation_timers;
    std::vector<int *> reserved_resources;


    void calc_income_stats(BaseStorage &base_storage);
    void calc_supply_per_frame(UnitStorage &unit_storage);

public:

    EconTracker(); 
    void init();
    void on_frame_update(BaseStorage &base_storage, UnitStorage &unit_storage);

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
    // allows current resources - reserved resources to go negative
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
    std::vector<std::vector<int>> build_order_sim(
        UnitStorage &unit_storage,
        BuildOrder *build_order
    );
   
};