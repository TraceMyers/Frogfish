#pragma once

#include "../utility/BWTimer.h"
#include <BWAPI.h>
#include <vector>

class EconTracker {

private:

    enum RESOURCES {MINERALS, GAS};

    enum FRAMESTUFF {FRAME_CT=5};

    BWAPI::Player self;
    BWTimer<void *> update_calc_timer;

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
    std::vector<BWTimer<void *> *> reservation_timers;
    std::vector<int *> reserved_resources;

public:

    EconTracker(int _calc_delay_seconds=8) : 
        calc_delay_seconds(_calc_delay_seconds),
        prev_total_minerals(50),
        prev_total_gas(0),
        prev_used_supply(8),
        reserved_minerals(0),
        reserved_gas(0),
        reserved_resource_ID(0),
        minerals_per_sec(0.0),
        gas_per_sec(0.0),
        supply_per_sec(0.0),
        sf_head(0),
        sf_prev(8)
    {
        update_calc_timer.start(calc_delay_seconds, 0);
    }

    void init() {self = Broodwar->self();}

    void on_frame_update() {
        update_calc_timer.on_frame_update();
        if (update_calc_timer.is_stopped()) {
            calc_stats();
            update_calc_timer.restart(); 
        }
        int reservation_ct = reservation_timers.size();
        std::vector<int> kill_res_IDs(reservation_ct);
        for (int i = 0; i < reservation_ct; ++i) {
            reservation_timers[i]->on_frame_update();
            if (reservation_timers[i]->is_stopped()) {
                kill_res_IDs.push_back(reservation_IDs[i]);
            }
        }
        for (auto _ID : kill_res_IDs) {
            end_reservation(_ID);
        }
        int x = 10;
        int y = 10;
        for (int i = 0; i < FRAME_CT; ++i) {
            Broodwar->drawTextScreen(x, y, "%d: %.3lf", i, supply_frames[i]);
            y += 10;
        }
    }

    // TODO: this probably needs to get more accurate in the future
    // 15 seconds is a long delay to get new economic information
    // maybe use 15 second frames, and just move the frame forward
    void calc_stats() {
        int cur_total_minerals = self->gatheredMinerals();
        int difference = cur_total_minerals - prev_total_minerals;
        minerals_per_sec = (double) difference / calc_delay_seconds;
        prev_total_minerals = cur_total_minerals;
        int cur_total_gas = self->gatheredGas();
        difference = cur_total_gas - prev_total_gas;
        gas_per_sec = (double) difference / calc_delay_seconds;
        prev_total_gas = cur_total_gas;
        int cur_used_supply = self->supplyUsed();
        difference = cur_used_supply - prev_used_supply;
        supply_frames[sf_head] = (double)difference / calc_delay_seconds;
        prev_used_supply = cur_used_supply;
        ++sf_head;
        if (sf_head == FRAME_CT) {
            sf_head = 0;
        }

    }

    // creates a curved average distribution of recent supply
    // for more accurate averaging
    double supply_frames_average() {
        double 
            long_term_supply_change = 0.0,
            distribution_factor = 0.35;
        for (int i = sf_head, moved_ct = 0; moved_ct < FRAME_CT; ++i, ++moved_ct)  {
            if (i == FRAME_CT) {
                i = 0;
            }
            if (supply_frames[i] == 0 && i != 0 && i != FRAME_CT - 1) {
                supply_frames[i] = (supply_frames[i - 1] + supply_frames[i + 1]) / 3.0;
            }
            long_term_supply_change += supply_frames[i] * distribution_factor;
            distribution_factor *= 0.7157;
        }
        return long_term_supply_change; //* 0.5 + supply_frames[0] * 0.5;
    }

    double get_minerals_per_sec() {return minerals_per_sec;}

    double get_gas_per_sec() {return gas_per_sec;}

    double get_supply_per_sec() {return supply_frames_average();}

    int get_free_minerals() {return self->minerals() - reserved_minerals;}

    int get_free_gas() {return self->gas() - reserved_gas;}

    // returns reference ID
    // allows current resources - reserved resources to go negative
    unsigned int make_reservation(int minerals, int gas, int reservation_seconds) {
        reserved_minerals += minerals;
        reserved_gas += gas;
        BWTimer<void *> *reserve_timer = new BWTimer<void *>();
        reserve_timer->start(reservation_seconds, 0);
        reservation_timers.push_back(reserve_timer); 
        reserved_resources.push_back(new int[2] {minerals, gas});
        reservation_IDs.push_back(reserved_resource_ID);
        reserved_resource_ID++;
        return reserved_resource_ID - 1;
    }

    bool reservation_alive(unsigned int ID) {
        auto it = std::find(reservation_IDs.begin(), reservation_IDs.end(), ID);
        if (it != reservation_IDs.end()) {
            return true;
        }
        return false;
    }

    // used internally and externally. Works for killing/canceling a res,
    // or for noting that the res is filled
    bool end_reservation(unsigned int ID) {
        auto it_rr = reserved_resources.begin();
        auto it_rt = reservation_timers.begin();
        auto it_rid = reservation_IDs.begin();
        for (;it_rr != reserved_resources.end(); ++it_rr, ++it_rt, ++it_rid) {
            if (*it_rid == ID) {
                reserved_minerals -= (*it_rr)[0];
                reserved_gas -= (*it_rr)[1];
                delete *it_rr;
                delete *it_rt;
                reserved_resources.erase(it_rr);
                reservation_timers.erase(it_rt);
                reservation_IDs.erase(it_rid);
                return true;
            }
        }
        return false;
    }

    bool extend_reservation(unsigned int ID, int seconds) {
        for (unsigned int i = 0; i < reservation_IDs.size(); ++i) {
            if (ID == reservation_IDs[i]) {
                int frames_left = reservation_timers[i]->get_frames_left();
                reservation_timers[i]->start(seconds, frames_left);
                return true;
            }
        }
        return false;
    }
};