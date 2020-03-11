#include "../FrogFish.h"
#include "Economy.h"
#include "../basic/Units.h"
#include "../basic/Bases.h"
#include "../utility/BWTimer.h"
#include <BWAPI.h>
#include <BWEM/bwem.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace Production;

// TODO: redo mining regression accounting for distance

namespace Production::Economy {

    namespace {
        enum RESOURCES {MINERALS, GAS};

        BWAPI::Player self;
        BWTimer update_calc_timer;

        const int
            SUPPLY_FRAME_SECONDS = 8,
            SUPPLY_FRAME_CT = 3,
            USABLE_BUFFER = 48;
        int
            reserved_minerals = 0,
            reserved_gas = 0,
            supply_frames[3] {0},
            larva_ct = 0,
            prev_supply;
        unsigned int 
            reserved_resource_ID = 1;
        double
            supply_per_frame = 0.0,
            minerals_per_frame = 0.0,
            gas_per_frame = 0.0,
            larva_per_frame = 0.0;
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

        std::vector<unsigned int> reservation_IDs;
        std::vector<BWTimer *> reservation_timers;
        std::vector<int *> reserved_resources;
        std::vector<std::vector<int>> build_order_sim_data;

        void estimate_income() {
            larva_ct = 0;
            minerals_per_frame = 0.0;
            gas_per_frame = 0.0;
            larva_per_frame = 0.0;
            for (auto &base : Basic::Bases::self_bases()) {
                larva_ct += Basic::Bases::larva(base).size();

                auto &minerals = base->Minerals();
                auto &base_depots = Basic::Bases::depots(base);
                int resource_depot_ct = base_depots.size();
                larva_per_frame += LPF_CONST * resource_depot_ct;

                int mineral_worker_ct = 0;
                for (auto &worker : Basic::Bases::workers(base)) {
                    Basic::Units::UnitData unit_data = Basic::Units::data(worker);
                    if (unit_data.u_task == Basic::Refs::UTASK::MINERALS) {
                        ++mineral_worker_ct;
                    }
                    else if (unit_data.u_task == Basic::Refs::UTASK::GAS) {
                        gas_per_frame += GPF_CONST;
                    }
                }
                int mineral_ct = minerals.size(); 

                double
                    min_sat = (double)mineral_worker_ct / mineral_ct,
                    min_sat_plus_1 = min_sat + 1,
                    min_sat_sq_fact = min_sat_plus_1 * min_sat_plus_1,
                    min_sat_cu_fact = min_sat_sq_fact * min_sat_plus_1;

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

        void estimate_supply_per_frame() {
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

        // TODO: Account for tech requirements
        std::vector<std::vector<int>> simulate(int sim_seconds=360) {
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
            unsigned cur_ID = (unsigned)BuildOrder::current_index();
            int
                extractor_drone_sink = 0,
                cur_make_ct = 0;

            std::vector<int> making_IDs;
            std::vector<BWAPI::UnitType> making_types;
            std::vector<int> making_frames_left;

            std::vector<std::vector<int>> ID_and_start_time;
            auto &self_units = Basic::Units::self_units();

            for (int i = 0; i < self_units.size(); ++i) {
                BWAPI::Unit u = self_units[i];
                int remaining_time = u->getRemainingBuildTime(); 
                if (u->getType() == BWAPI::UnitTypes::Zerg_Egg) {
                    making_types.push_back(u->getBuildType());
                    making_frames_left.push_back(remaining_time);
                }
                else if (u->getType() == BWAPI::UnitTypes::Zerg_Hatchery && remaining_time > 0) {
                    making_types.push_back(BWAPI::UnitTypes::Zerg_Hatchery);
                    making_frames_left.push_back(remaining_time);
                }
            }
            
            int seconds_passed = 0;
            while(cur_ID < BuildOrder::size() && seconds_passed < sim_seconds) {
                auto &item = BuildOrder::get(cur_ID);
                const BuildOrder::Item::ACTION &action = item.action();

                // TODO: Account for 200 supply max
                const BWAPI::UnitType &unit_type = item.unit_type();
                int 
                    min_cost = item.mineral_cost(),
                    gas_cost = item.gas_cost(),
                    supply_cost = item.supply_cost(),
                    larva_cost = item.larva_cost(),
                    make_ct = item.count();

                if (
                    min_cost <= minerals 
                    && gas_cost <= gas 
                    && (supply_cost <= 0 || supply_cost <= supply_total - supply_used)
                    && (larva_cost == 0 || larva >= 1)
                ) {
                    minerals -= min_cost;
                    gas -= gas_cost;
                    supply_used += supply_cost;
                    larva -= larva_cost;
                    ++cur_make_ct;

                    if (
                        action == BuildOrder::Item::MAKE
                        || action == BuildOrder::Item::MORPH
                        || action == BuildOrder::Item::BUILD
                    ) {
                        if (action == BuildOrder::Item::BUILD) {
                            mps -= add_drone_mps;
                        }
                        making_IDs.push_back(cur_ID);
                        making_types.push_back(unit_type);
                        making_frames_left.push_back(unit_type.buildTime() + USABLE_BUFFER);
                    }
                    else if (action == BuildOrder::Item::CANCEL) {
                        int cancel_ID = item.cancel_index();
                        auto ID_it = making_IDs.begin();
                        auto type_it = making_types.begin();
                        auto frames_it = making_frames_left.begin();
                        for ( ; ID_it != making_IDs.end(); ++ID_it, ++type_it, ++frames_it) {
                            if (*(ID_it) == cancel_ID) {
                                if (unit_type.whatBuilds().first == BWAPI::UnitTypes::Zerg_Drone) {
                                    mps += add_drone_mps;
                                }
                                making_IDs.erase(ID_it);
                                making_types.erase(type_it);
                                making_frames_left.erase(frames_it);
                                break;
                            }
                        }
                    }
                    ID_and_start_time.push_back(std::vector<int> {(int)cur_ID, seconds_passed});
                    if (cur_make_ct == make_ct) {
                        cur_make_ct = 0;
                        ++cur_ID;
                    }
                }
                else {
                    ++seconds_passed;
                    minerals += mps;
                    gas += gps;
                    larva += lps;

                    auto ID_it = making_IDs.begin();
                    auto type_it = making_types.begin();
                    auto frames_it = making_frames_left.begin();
                    for ( ; ID_it != making_IDs.end(); ++ID_it, ++type_it, ++frames_it) {
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

                            ID_it = making_IDs.erase(ID_it);
                            type_it = making_types.erase(type_it);
                            frames_it = making_frames_left.erase(frames_it);
                        }
                    }
                }
            }
            return ID_and_start_time;
        }
    }

    void init() {
        self = Broodwar->self(); 
        prev_supply = self->supplyUsed();
        supply_frame_timer.start(SUPPLY_FRAME_SECONDS, 0);
    }

    void on_frame_update() {
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
        estimate_income();
        supply_frame_timer.on_frame_update();
        if (supply_frame_timer.is_stopped()) {
            estimate_supply_per_frame();
            supply_frame_timer.restart();
        }
        build_order_sim_data = simulate();
    }

    double get_minerals_per_frame() {return minerals_per_frame;}

    double get_gas_per_frame() {return gas_per_frame;}

    double get_larva_per_frame() {return larva_per_frame;}

    double get_supply_per_frame() {return supply_per_frame;}

    double get_minerals_per_sec() {return minerals_per_frame * 24;}

    double get_gas_per_sec() {return gas_per_frame * 24;}

    double get_larva_per_sec() {return larva_per_frame * 24;}

    double get_supply_per_sec() {return supply_per_frame * 24;}

    int get_free_minerals() {return self->minerals() - reserved_minerals;}

    int get_free_gas() {return self->gas() - reserved_gas;}

    // returns reference ID
    // allows current resources - reserved resources to go negative
    unsigned int make_reservation(int minerals, int gas, int reservation_seconds) {
        printf("reservation for %d minerals made\n", minerals);
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

    // something should have gone wrong if this returns true to a concerned party;
    // said party should be regularly checking on its reservation's time left
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
                printf("reservation for %d minerals canceled\n", (*it_rr)[0]);
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

    const std::vector<std::vector<int>> &sim_data() {
        return build_order_sim_data;
    }

    void print_sim_data() {
        printf("------------------\n---------Sim Data:---------\n------------------\n\n");
        for (auto &item : build_order_sim_data) {
            int ID = item[0];
            int time = item[1];
            printf("Time: %d seconds\n", time);
            BuildOrder::print_item((unsigned)ID);
        }
    }
}