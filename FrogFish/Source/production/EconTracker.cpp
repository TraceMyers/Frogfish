#include "../FrogFish.h"
#include "EconTracker.h"
#include "MakeQueue.h"
#include "../unitdata/UnitStorage.h"
#include "../unitdata/BaseStorage.h"
#include "../unitdata/FrogBase.h"
#include "../unitdata/FrogUnit.h"
#include "../utility/BWTimer.h"
#include <BWAPI.h>
#include <BWEM/bwem.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>

// TODO: need to reserve minerals and gas separately
EconTracker::EconTracker() : 
    reserved_minerals(0),
    reserved_gas(0),
    larva_ct(0),
    reserved_resource_ID(0),
    minerals_per_frame(0.0),
    gas_per_frame(0.0),
    larva_per_frame(0.0),
    supply_per_frame(0.0),
    supply_frames()
{}

void EconTracker::init() {
    self = Broodwar->self(); 
    prev_supply = self->supplyUsed();
    supply_frame_timer.start(SUPPLY_FRAME_SECONDS, 0);
}

void EconTracker::on_frame_update(BaseStorage &base_storage, UnitStorage &unit_storage) {
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
	calc_income_stats(base_storage);
    supply_frame_timer.on_frame_update();
    if (supply_frame_timer.is_stopped()) {
        calc_supply_per_frame(unit_storage);
        supply_frame_timer.restart();
    }

}

// keep current coefs for normal mining, but need a coef for distance
// greater than some constant to calc long distance mining
void EconTracker::calc_income_stats(BaseStorage &base_storage) {
    larva_ct = 0;
	minerals_per_frame = 0.0;
    gas_per_frame = 0.0;
    larva_per_frame = 0.0;
    for (auto &base : base_storage.get_self_bases()) {
        // larva
        larva_ct += base->get_larva_ct();

        auto &minerals = base->get_minerals();
		int resource_depot_ct = base->get_resource_depot_ct();
		FUnit main_hatch;
		if (resource_depot_ct > 0) {
			double min_dist = 100000000.0;
			for (auto &hatch : base->get_resource_depots()) {
				int dist = 0;
                BWAPI::Position hatch_pos = hatch->get_pos();
				for (auto &mineral : minerals) {
					dist += hatch_pos.getApproxDistance(mineral->Pos());
				}
                if (dist < min_dist) {
                    main_hatch = hatch;
                    min_dist = dist;
                }
                // larva per frame
                larva_per_frame += LPF_CONST;
			}
		}
        else {
            continue;
        }
        int 
            mineral_worker_ct = 0;
        for (auto &worker : base->get_workers()) {
            if (worker->f_task == FrogUnit::MINE_MINERALS) {
                ++mineral_worker_ct;
            }
            // gas per frame
            else if (worker->f_task == FrogUnit::MINE_GAS) {
                gas_per_frame += GPF_CONST;
            }
        }
        int mineral_ct = minerals.size(); 

		double
			min_sat = (double)mineral_worker_ct / mineral_ct,
			min_sat_plus_1 = min_sat + 1,
			min_sat_sq_fact = min_sat_plus_1 * min_sat_plus_1,
			min_sat_cu_fact = min_sat_sq_fact * min_sat_plus_1;

        // minerals per frame
        minerals_per_frame += 
			(
				MPF_INTERCEPT
				+ min_sat * MPF_SATURATION_COEF
				+ min_sat_sq_fact * MPF_SATURATION_SQ_COEF
				+ min_sat_cu_fact * MPF_SATURATION_CU_COEF
			) 
			* mineral_worker_ct;
    }
}

void EconTracker::calc_supply_per_frame(UnitStorage &unit_storage) {
    double supply_per_frame_period = 0.0;
    int supply_diff = self->supplyUsed() - prev_supply;
    prev_supply = self->supplyUsed();
    for (int i = 0; i < SUPPLY_FRAME_CT; ++i) {
        if (i < SUPPLY_FRAME_CT - 1) {
            supply_frames[i + 1] = supply_frames[i];
        }
        if (i == 0) {
            supply_frames[i] = supply_diff;
        }
        supply_per_frame_period += supply_frames[i] * SUPPLY_FRAME_POLY_FACT;
    } 
    supply_per_frame = supply_per_frame_period / (SUPPLY_FRAME_SECONDS * 24);
}
    
double EconTracker::get_minerals_per_frame() {return minerals_per_frame;}

double EconTracker::get_gas_per_frame() {return gas_per_frame;}

double EconTracker::get_larva_per_frame() {return larva_per_frame;}

double EconTracker::get_supply_per_frame() {return supply_per_frame;}

double EconTracker::get_minerals_per_sec() {return minerals_per_frame * 24;}

double EconTracker::get_gas_per_sec() {return gas_per_frame * 24;}

double EconTracker::get_larva_per_sec() {return larva_per_frame * 24;}

double EconTracker::get_supply_per_sec() {return supply_per_frame * 24;}

int EconTracker::get_free_minerals() {return self->minerals() - reserved_minerals;}

int EconTracker::get_free_gas() {return self->gas() - reserved_gas;}

// returns reference ID
// allows current resources - reserved resources to go negative
unsigned int EconTracker::make_reservation(int minerals, int gas, int reservation_seconds) {
    reserved_minerals += minerals;
    reserved_gas += gas;
    BWTimer *reserve_timer = new BWTimer();
    reserve_timer->start(reservation_seconds, 0);
    reservation_timers.push_back(reserve_timer); 
    reserved_resources.push_back(new int[2] {minerals, gas});
    reservation_IDs.push_back(reserved_resource_ID);
    reserved_resource_ID++;
    return reserved_resource_ID - 1;
}

bool EconTracker::reservation_alive(unsigned int ID) {
    auto it = std::find(reservation_IDs.begin(), reservation_IDs.end(), ID);
    if (it != reservation_IDs.end()) {
        return true;
    }
    return false;
}

// used internally and externally. Works for killing/canceling a res,
// or for noting that the res is filled
bool EconTracker::end_reservation(unsigned int ID) {
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

bool EconTracker::extend_reservation(unsigned int ID, int seconds) {
    for (unsigned int i = 0; i < reservation_IDs.size(); ++i) {
        if (ID == reservation_IDs[i]) {
            int frames_left = reservation_timers[i]->get_frames_left();
            reservation_timers[i]->start(seconds, frames_left);
            return true;
        }
    }
    return false;
}

// TODO: 
//      - if using reservations, need to factor them here
//      - upgrades

// Assumes:
    // - things are made immediately when they can be
    // - that when an extractor finishes, 3 drones go to gas, all others mine minerals
    // - drones mine minerals at bases with a drone-to-mineral ratio of 1, 
    // which is pretty accurate for ratios between ~0.4 and ~1.6
    // - drones are available to make buildings, and that they will be taken off
    // of mineral mining. 
    // - that units being morphed from other units have those units available to
    // morph from
    // - that larva are spawn continuously from every hatchery (no sitting on larva)
// - Cancels do not support more than 1 cancellation per ID or passed-in making type
// - Becomes fairly inaccurate if a cancellation is in the build order wherein the
// building finishes before the cancellation can occur.
std::vector<std::vector<int>> EconTracker::build_order_sim(
    UnitStorage &unit_storage,
    BuildOrder *build_order,
    MakeQueue &make_queue
) {
    std::vector<BWAPI::UnitType> making_types_in;
    std::vector<int> making_frames_left_in;
    for (auto &unit : unit_storage.self_units()) {
        FUnit u = unit.second;
        if (u->f_type == FrogUnit::EGG) {
            making_types_in.push_back(u->bwapi_u()->getBuildType());
            making_frames_left_in.push_back(u->bwapi_u()->getRemainingBuildTime());
        }
    }
    std::vector<std::vector<int>> seconds_until_make;
    auto &make_deque = make_queue.get_queue();
    double 
        minerals = get_free_minerals(),
        gas = get_free_gas(),
        larva = larva_ct,
        supply_used = self->supplyUsed(),
        supply_total = self->supplyTotal(),
        mps = minerals_per_frame * 24,
        gps = gas_per_frame * 24,
        lps = larva_per_frame * 24,
        add_drone_mps = MPF_SIMPLE_CONST * 24,
        add_drone_gps = GPF_CONST * 24,
        add_hatch_lps = LPF_CONST * 24;
    unsigned cur_ID;
    int
        ID_start = build_order->cur_item,
        extractor_drone_sink = 0,
        cur_ID_make_ct = 0,
        make_deque_i = 0;
    std::vector<int> making_IDs;
    std::vector<BWAPI::UnitType> making_types;
    std::vector<int> making_frames_left;

    int seconds_passed = 0;
    for (cur_ID = ID_start; cur_ID < build_order->size(); ) {
        const BuildItem &item = build_order->get(cur_ID);
        
        if (
            make_deque_i < make_deque.size()
            || item.build_type == BuildItem::MAKE_UNIT 
            || item.build_type == BuildItem::BUILD
        ) {
            BWAPI::UnitType u_type;
            if (make_deque_i < make_deque.size()) {
                // printf("deque_i : %d\n",make_deque_i);
                u_type = make_deque[make_deque_i];
            }
            else {
               u_type = item.make_type;
            }
            int 
                min_cost = u_type.mineralPrice(),
                gas_cost = u_type.gasPrice(),
                supply_cost = u_type.supplyRequired(),
                make_ct = item.count;
            bool from_larva = false;
            for (int i = 0; i < MakeQueue::MKQ_UNIT_TYPE_CT; ++i) {
                if (u_type == MakeQueue::types[i]) {
                    from_larva = true;
                    break;
                }
            }

            if (
                min_cost <= minerals 
                && gas_cost <= gas 
                && supply_cost <= (supply_total - supply_used)
                && (!from_larva || larva >= 1)
            ) {
                // printf("seconds passed: %d\n", seconds_passed);
                // printf("minerals: %.2lf\n", minerals);
                // printf("gas: %.2lf\n", minerals);
                // printf("can make %s\n", u_type.c_str());
                // printf("id : %d\n", cur_ID);
                minerals -= min_cost;
                gas -= gas_cost;
                supply_used -= supply_cost;

                if (u_type.isBuilding()) {
                    mps -= add_drone_mps;
                    supply_used -= 2;
                }
                making_IDs.push_back(cur_ID);
                making_types.push_back(u_type);
                making_frames_left.push_back(u_type.buildTime() + FINISH_TIME);
                if (from_larva) {
                    --larva;
                }
                if (make_deque_i < make_deque.size()) {
                    ++make_deque_i;
                }
                else {
                    seconds_until_make.push_back(std::vector<int> {(int)cur_ID, seconds_passed});
                    ++cur_ID_make_ct;
                    if (cur_ID_make_ct == make_ct) {
                        cur_ID_make_ct = 0;
                        ++cur_ID;
                    }
                }
            }
            else {
                ++seconds_passed;
                minerals += mps;
                gas += gps;
                larva += lps;
                // printf("seconds passed: %d, minerals: %.2lf, mps: %.2lf\n", 
                //     seconds_passed, minerals, mps);
                auto ID_it = making_IDs.begin();
                auto type_it = making_types.begin();
                auto frames_it = making_frames_left.begin();
                for ( ; ID_it != making_IDs.end(); ++ID_it, ++type_it, ++frames_it) {
                    *(frames_it) -= 24;
                    if (*(frames_it) <= 0) {
                        BWAPI::UnitType &finished_type = *(type_it);
                        int finished_ID = *(ID_it);

                        supply_total += finished_type.supplyProvided();
                        if (finished_type == BWAPI::UnitTypes::Zerg_Extractor) {
                            extractor_drone_sink += 3;
                        }
                        else if (finished_type == BWAPI::UnitTypes::Zerg_Drone) {
                            if (extractor_drone_sink > 0) {
                                gps += add_drone_gps;
                                --extractor_drone_sink;
                            }
                            else {
                                mps += add_drone_mps;
                            }
                        }
                        else if (finished_type == BWAPI::UnitTypes::Zerg_Hatchery) {
                            lps += add_hatch_lps;
                        }

                        ID_it = making_IDs.erase(ID_it);
                        type_it = making_types.erase(type_it);
                        frames_it = making_frames_left.erase(frames_it);
                        if (ID_it == making_IDs.end()) {break;}
                    }
                }
                type_it = making_types_in.begin();
                frames_it = making_frames_left_in.begin();
                for ( ; type_it != making_types_in.end(); ++type_it, ++frames_it) {
                    *(frames_it) -= 24;
                    if (*(frames_it) <= 0) {
                        BWAPI::UnitType &finished_type = *(type_it);

                        supply_total += finished_type.supplyProvided();
                        if (finished_type == BWAPI::UnitTypes::Zerg_Extractor) {
                            extractor_drone_sink += 3;
                        }
                        else if (finished_type == BWAPI::UnitTypes::Zerg_Drone) {
                            if (extractor_drone_sink > 0) {
                                gps += add_drone_gps;
                                --extractor_drone_sink;
                            }
                            else {
                                mps += add_drone_mps;
                            }
                        }
                        else if (finished_type == BWAPI::UnitTypes::Zerg_Hatchery) {
                            lps += add_hatch_lps;
                        }

                        type_it = making_types_in.erase(type_it);
                        frames_it = making_frames_left_in.erase(frames_it);
                        if (type_it == making_types_in.end()) {break;}
                    }
                }
            }
        }
        else if (item.build_type == BuildItem::CANCEL) {
            int cancel_ID = item.required_i;
            auto ID_it = making_IDs.begin();
            auto type_it = making_types.begin();
            auto frames_it = making_frames_left.begin();
            bool found_cancel = false;
            for ( ; ID_it != making_IDs.end(); ++ID_it, ++type_it, ++frames_it) {
                if (*(ID_it) == cancel_ID) {
                    BWAPI::UnitType &cancel_type = *type_it;
                    minerals += cancel_type.mineralPrice() * 0.75;
                    gas += cancel_type.gasPrice() * 0.75;
                    supply_used -= cancel_type.supplyRequired();

                    making_IDs.erase(ID_it);
                    making_types.erase(type_it);
                    making_frames_left.erase(frames_it);
                    found_cancel = true;

                    if (item.make_type.isBuilding()) {
                        mps += add_drone_mps;
                    }
                    break;
                }
            }
            if (!found_cancel) {
                type_it = making_types_in.begin();
                for ( ; type_it < making_types_in.end(); ++type_it) {
                    if (item.make_type == *type_it) {
                        const BWAPI::UnitType &cancel_type = item.make_type;
                        minerals += cancel_type.mineralPrice() * 0.75;
                        gas += cancel_type.gasPrice() * 0.75;
                        supply_used -= cancel_type.supplyRequired();
                        making_types_in.erase(type_it);

                        if (item.make_type.isBuilding()) {
                            mps += add_drone_mps;
                            supply_used += 2;
                        }
                        found_cancel = true;
                        break;
                    }
                }
            }
            // advances to the next item even if a cancellation isn't found in order
            // to not go into an infinite loop if a cancellation was scheduled incorrectly
            ++cur_ID;
        }
        if (seconds_passed > 360) {
            break;
        }
    }
    return seconds_until_make;
}