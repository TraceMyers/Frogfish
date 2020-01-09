#include<BWAPI.h>
#include "../utility/BWTimer.h"

class EconStats {

private:

    int calc_delay_seconds;

    BWAPI::Player self;
    BWTimer<void *> update_calc_timer;

    int prev_self_total_minerals;
    int prev_self_total_gas;
    int prev_self_used_supply;
    double self_minerals_per_sec;
    double self_gas_per_sec;
    double self_supply_per_sec;

    // don't worry about enemy stuff for a while
    double seen_enemy_minerals_per_sec;
    double seen_enemy_gas_per_sec;
    double estim_enemy_minerals_per_sec;
    double estim_enemy_gas_per_sec;

public:

    EconStats(int _calc_delay_seconds=15) : 
        calc_delay_seconds(_calc_delay_seconds)
    {
        update_calc_timer.start(calc_delay_seconds, 0);
    }

    void init() {self = Broodwar->self();}

    void on_frame_update() {
        update_calc_timer.on_frame_update();
        if (update_calc_timer.is_stopped()) {
            calc_self_stats();
            update_calc_timer.restart(); 
        }
    }

    void calc_self_stats() {
        int cur_self_total_minerals = self->gatheredMinerals();
        int difference = cur_self_total_minerals - prev_self_total_minerals;
        self_minerals_per_sec = (double) difference / calc_delay_seconds;
        prev_self_total_minerals = cur_self_total_minerals;
        int cur_self_total_gas = self->gatheredGas();
        difference = cur_self_total_gas - prev_self_total_gas;
        self_gas_per_sec = (double) difference / calc_delay_seconds;
        prev_self_total_gas = cur_self_total_gas;
        int cur_self_used_supply = self->supplyUsed();
        difference = cur_self_used_supply - prev_self_used_supply;
        self_supply_per_sec = (double) difference / calc_delay_seconds;
        prev_self_used_supply = cur_self_used_supply;

        printf(
            "mps: %.2lf\ngps: %.2lf\nsps: %.2lf\n", 
            self_minerals_per_sec,
            self_gas_per_sec,
            self_supply_per_sec
        );
    }

    double get_self_minerals_per_sec() {return self_minerals_per_sec;}

    double get_self_gas_per_sec() {return self_gas_per_sec;}
};