#include "../FrogFish.h"
#include "Economy.h"
#include "../basic/Units.h"
#include "../basic/Bases.h"
#include "../utility/BWTimer.h"
#include "../test/TestMessage.h"
#include <BWAPI.h>
#include <BWEM/bwem.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include "Windows.h"

using namespace Production;

// TODO: redo mining regression accounting for distance
// TODO: revise reservations to use build order ID's

namespace Production::Economy {

    namespace {
        enum RESOURCES {MINERALS, GAS};

        BWAPI::Player self;
        BWTimer update_calc_timer;

        // TODO: make mpf/gpf dependent on more factors
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

        std::vector<std::pair<int, int>> sim_data;
        std::vector<int> sim_making_IDs;
        std::vector<int> sim_making_frames_left;
        std::vector<BWAPI::UnitType> sim_making_types;
        int 
            sim_seconds_until_supply_block,
            sim_ID_at_supply_block,
            sim_incoming_supply;
        bool 
            sim_supply_block_flag,
            sim_just_added_overlord;
        int
            sim_supply_used,
            sim_supply_total;
        float
            sim_larva,
            sim_minerals,
            sim_gas,
            sim_mps,
            sim_gps,
            sim_lps,
            sim_add_drone_mps,
            sim_add_drone_gps,
            sim_add_hatch_lps;

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
                    /*
                    // TODO: Use this
                    Basic::Units::UnitData unit_data = Basic::Units::data(worker);
                    if (unit_data.u_task == Basic::Refs::UTASK::MINERALS) {
                        ++mineral_worker_ct;
                    }
                    else if (unit_data.u_task == Basic::Refs::UTASK::GAS) {
                        gas_per_frame += GPF_CONST;
                    }
                    */
                   ++mineral_worker_ct;
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
                DBGMSG("minerals per frame: %.2f\n", minerals_per_frame);
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

        // TODO: magic numbers in sim functions
        void sim_init() {
            sim_data.clear();
            sim_making_IDs.clear();
            sim_making_types.clear();
            sim_making_frames_left.clear();

            auto &self_units = Basic::Units::self_units();
            int NON_SIM_MAKING_ID = -1000;

            sim_minerals = get_free_minerals();
            sim_gas = get_free_gas();
            sim_larva = (larva_ct == 0 ? 0.5 : larva_ct); // TODO: better? starting at 0.5 more accurate
            sim_supply_used = self->supplyUsed();
            sim_supply_total = self->supplyTotal();
            sim_mps = minerals_per_frame * 24;
            sim_gps = gas_per_frame * 24;
            sim_lps = larva_per_frame * 24;
            sim_incoming_supply = 0;

            for (int i = 0; i < self_units.size(); ++i) {
                BWAPI::Unit u = self_units[i];
                int remaining_time = u->getRemainingBuildTime(); 
                if (u->getType() == BWAPI::UnitTypes::Zerg_Egg) {
                    sim_making_IDs.push_back(NON_SIM_MAKING_ID);
                    const BWAPI::UnitType &build_type = u->getBuildType();
                    if (build_type == BWAPI::UnitTypes::Zerg_Overlord) {
                        sim_incoming_supply += 16;
                    }
                    sim_making_types.push_back(build_type);
                    sim_making_frames_left.push_back(remaining_time);
                    ++NON_SIM_MAKING_ID;
                }
                else if (u->getType() == BWAPI::UnitTypes::Zerg_Hatchery && remaining_time > 0) {
                    sim_incoming_supply += BWAPI::UnitTypes::Zerg_Hatchery.supplyProvided();
                    sim_making_IDs.push_back(NON_SIM_MAKING_ID);
                    sim_making_types.push_back(BWAPI::UnitTypes::Zerg_Hatchery);
                    sim_making_frames_left.push_back(remaining_time);
                    ++NON_SIM_MAKING_ID;
                }
            }
        }

        bool sim_can_make_item(const BuildOrder::Item& item, int cur_ID) {
            const BuildOrder::Item::ACTION &action = item.action();
            int 
                min_cost = item.mineral_cost(),
                gas_cost = item.gas_cost(),
                supply_cost = item.supply_cost(),
                larva_cost = item.larva_cost();

            sim_supply_block_flag = (supply_cost > 0 && supply_cost > sim_supply_total - sim_supply_used);
            if (
                !sim_supply_block_flag
                && min_cost <= sim_minerals
                && gas_cost <= sim_gas 
                && (larva_cost == 0 || sim_larva >= 1)
            ) {
                return true;
            }
            return false;
        }

        bool sim_remove_item(int ID) {
            bool remove_success = false;
            for (unsigned i = 0; i < sim_making_IDs.size(); ++i) {
                if (ID == sim_making_IDs[i]) {
                    sim_making_IDs.erase(sim_making_IDs.begin() + i);
                    sim_making_types.erase(sim_making_types.begin() + i);
                    sim_making_frames_left.erase(sim_making_frames_left.begin() + i);
                    remove_success = true;
                    break;
                }
            }
            return remove_success;
        }

        void sim_add_item(int ID, int seconds_passed) {
            auto &item = BuildOrder::get(ID);
            auto &type = item.unit_type();
            sim_making_IDs.push_back(ID);
            sim_making_types.push_back(type);
            if (type == BWAPI::UnitTypes::Zerg_Drone) {
                sim_making_frames_left.push_back(item.unit_type().buildTime() + 72);
            }
            else {
                sim_making_frames_left.push_back(item.unit_type().buildTime() + 10);
            }
        }

        // subtract mps when make building
        void sim_make_item(const BuildOrder::Item& item, int cur_ID, int seconds_passed) {
            sim_data.push_back(std::pair<int, int>(cur_ID, seconds_passed));
            const auto &action = item.action();
            auto &type = item.unit_type();
            if (item.supply_cost() != 0) {
                sim_add_item(cur_ID, seconds_passed);
                int provided_supply = type.supplyProvided();
                if (provided_supply > 0) {
                    sim_incoming_supply += provided_supply;
                    if (action == BuildOrder::Item::BUILD) {
                        sim_supply_used -= 2;
                    }
                }
                else {
                    sim_supply_used += item.supply_cost();
                }
            }
            else if (action == BuildOrder::Item::CANCEL) {
                bool successful_cancel = sim_remove_item(item.cancel_index());
                if (successful_cancel) {
                    sim_supply_used += item.supply_cost();
                    int provided_supply = type.supplyProvided();
                    sim_incoming_supply -= provided_supply;
                }
                else {
                    printf("econ sim - unsuccessful cancel of item\n***\n");
                    BuildOrder::print_item(cur_ID);
                    printf("***\n");
                    return;
                }
            }
            sim_minerals -= item.mineral_cost();
            sim_gas -= item.gas_cost();
            sim_larva -= item.larva_cost();
        }

        void sim_advance_making_items() {
            auto &frames_left_it = sim_making_frames_left.begin();
            auto &IDs_it = sim_making_IDs.begin();
            auto &types_it = sim_making_types.begin();
            while (frames_left_it < sim_making_frames_left.end()) {
                (*frames_left_it) -= 24;
                int making_frames_left = *frames_left_it;
                if (making_frames_left <= 0) {
                    auto& type = *types_it;
                    int supply_provided = type.supplyProvided();
                    if (supply_provided > 0) {
                        sim_supply_total += supply_provided;
                        sim_incoming_supply -= supply_provided;
                    }
                    if (type == BWAPI::UnitTypes::Zerg_Drone) {
                        // TODO: extractor sink
                        sim_mps += sim_add_drone_mps;
                    }
                    else if (type == BWAPI::UnitTypes::Zerg_Hatchery) {
                        sim_lps += sim_add_hatch_lps;
                    }
                    frames_left_it = sim_making_frames_left.erase(frames_left_it);
                    IDs_it = sim_making_IDs.erase(IDs_it);
                    types_it = sim_making_types.erase(types_it);
                    if (frames_left_it == sim_making_frames_left.end()) { break; }
                }
                ++frames_left_it;
                ++IDs_it;
                ++types_it;
            }
        }

        // TODO: Account for tech requirements
        void simulate_build_order(int sim_seconds=360) {
            const int 
                OVERLORD_MINERAL_COST = 100,
                OVERLORD_SUPPLY_PROVIDED = 16,
                OVERLORD_BUILD_TIME = 600 / 24,
                SUPPLY_MAX = 400;
            int seconds_passed = 0;
            unsigned cur_ID = (unsigned)BuildOrder::current_index();
            sim_seconds_until_supply_block = -1;
            sim_ID_at_supply_block = -1;
            sim_add_drone_mps = MPF_SIMPLE_CONST * 24;
            sim_add_drone_gps = GPF_CONST * 24;
            sim_add_hatch_lps = LPF_CONST * 24;

            sim_init();
            
            while (cur_ID < BuildOrder::size() && seconds_passed < sim_seconds) {
                auto &item = BuildOrder::get(cur_ID);
                if (sim_can_make_item(item, cur_ID)) {
                    sim_make_item(item, cur_ID, seconds_passed);
                    ++cur_ID;
                }
                else {
                    //DBGMSG("sim supply block flag: %d", sim_supply_block_flag);
                    //DBGMSG("sim incoming supply: %d", sim_incoming_supply);
                    if (   
                        sim_supply_block_flag 
                        && sim_incoming_supply <= 0 
                    ) {
                        sim_seconds_until_supply_block = seconds_passed;
                        sim_ID_at_supply_block = cur_ID;
                        break;
                    }
                    sim_advance_making_items();
                    sim_minerals += sim_mps;
                    sim_gas += sim_gps;
                    sim_larva += sim_lps;
                    ++seconds_passed;
                }
            }
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
        simulate_build_order();
        /*
        if (sim_data.size() > 0) {
            print_sim_data();
        }
        */
        sim_just_added_overlord = false;
    }

    void add_delay_to_build_order_sim(unsigned start_index, int delay_seconds, bool push_indices) {
        for (unsigned i = start_index; i < sim_data.size(); ++i) {
            auto& sim_item = sim_data[i];
            if (push_indices) {
                sim_item.first += 1;
            }
            sim_item.second += delay_seconds;
        }
    }

    void sim_set_just_added_overlord_flag_true() {
        sim_just_added_overlord = true;
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

    bool supply_blocked() {return self->supplyUsed() >= self->supplyTotal();}

    // returns reference ID
    // allows current resources - reserved resources to go negative
    void make_reservation(unsigned ID, int reservation_seconds) {
        auto &item = BuildOrder::get(ID);
        reserved_minerals += item.mineral_cost();
        reserved_gas += item.gas_cost();
        BWTimer *reserve_timer = new BWTimer();
        reserve_timer->start(reservation_seconds, 0);
        reservation_timers.push_back(reserve_timer); 
        reserved_resources.push_back(new int[2] {item.mineral_cost(), item.gas_cost()});
        reservation_IDs.push_back(ID);
    }

    // something should have gone wrong if this returns true to a concerned party;
    // said party should be regularly checking on its reservation's time left
    bool reservation_alive(unsigned ID) {
        auto it = std::find(reservation_IDs.begin(), reservation_IDs.end(), ID);
        if (it != reservation_IDs.end()) {
            return true;
        }
        return false;
    }

    // used internally and externally. Works for killing/canceling a res,
    // or for noting that the res is filled
    bool end_reservation(unsigned ID) {
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

    bool extend_reservation(unsigned ID, int seconds) {
        for (unsigned int i = 0; i < reservation_IDs.size(); ++i) {
            if (ID == reservation_IDs[i]) {
                int frames_left = reservation_timers[i]->get_frames_left();
                reservation_timers[i]->start(seconds, frames_left);
                return true;
            }
        }
        return false;
    }

    int seconds_until_supply_blocked() {
        return sim_seconds_until_supply_block;
    }

    int build_order_ID_at_supply_block() {
        return sim_ID_at_supply_block;
    }

    const std::vector<std::pair<int, int>> &get_sim_data() {
        return sim_data;
    }

    void print_sim_data(int limit) {
        int count = 0;
        printf("---------------------------\n---------Sim Data:---------\n---------------------------\n\n");
        for (auto &item : sim_data) {
            int ID = item.first;
            int time = item.second;
            printf("Time: %d seconds\n", time);
            BuildOrder::print_item((unsigned)ID);
            ++count;
            if (count == limit) { break; }
        }
    }
}