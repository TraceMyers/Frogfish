#include "Overlords.h"
#include "MakeQueue.h"
#include "MorphQueue.h"
#include "BuildQueue.h"
#include "../data/BaseStorage.h"
#include "../data/EconStats.h"
#include <BWAPI.h>
#include <vector>
#include <deque>

#define OVERLORD_SUPPLY_PROVIDED 16
#define OVERLORD_MAKE_SEC 25.0
#ifndef LARVA_SPAWN_SEC 
#define LARVA_SPAWN_SEC 14.166
#endif

// just using make_queue for now    
// not giving any preferences to where units are made
int get_overlord_make_ct(
    MakeQueue &make_queue, 
    MorphQueue &morph_queue, 
    BuildQueue &build_queue,
    BaseStorage &base_storage,
    EconStats &econ_stats
) {
    BWAPI::Player self = Broodwar->self();
    const std::deque<BWAPI::UnitType> &makes = make_queue.get_queue();
    const std::vector<FBase> &self_bases = base_storage.get_self_bases();
    double 
        mps = econ_stats.get_self_minerals_per_sec(),
        gps = econ_stats.get_self_gas_per_sec();
    int 
        used_supply = self->supplyUsed(),
        supply_total = self->supplyTotal(),
        minerals = self->minerals(),
        gas = self->gas(),
        larva = 0,
        makes_size = makes.size(),
        base_ct = self_bases.size();
    for (auto &base : self_bases) {
        larva += base->get_larva_ct();
    }
    
    int i = 0;
    while (minerals > 0 && gas >= 0 && larva > 0 && i < makes_size) {
        BWAPI::UnitType unit_type = makes[i];
        used_supply += unit_type.supplyRequired();
        supply_total += unit_type.supplyProvided();
        minerals -= unit_type.mineralPrice();
        gas -= unit_type.gasPrice();
        larva -= 1;
        i++;
    }
    if (used_supply >= supply_total) {
        double make_overlord_ct = 
            (double)(used_supply - supply_total) / OVERLORD_SUPPLY_PROVIDED;
        double ceiling = ceil(make_overlord_ct);
        make_overlord_ct = (ceiling <= 0.0 ? 1.0 : ceiling);
        return (int)make_overlord_ct;
    }
    minerals += (int)(mps * OVERLORD_MAKE_SEC);
    larva += (int)round((OVERLORD_MAKE_SEC / LARVA_SPAWN_SEC) * base_ct);
    gas += (int)(gas * OVERLORD_MAKE_SEC);
    i = 0;
    while (minerals > 0 && gas >= 0 && larva > 0) {
        BWAPI::UnitType unit_type = makes[i];
        used_supply += unit_type.supplyRequired();
        supply_total += unit_type.supplyProvided();
        minerals -= unit_type.mineralPrice();
        gas -= unit_type.gasPrice();
        larva -= 1;
        i++;
        if (i == makes_size) {
            i = 0;
        }
    }
    if (used_supply >= supply_total) {
        double make_overlord_ct = 
            (double)(used_supply - supply_total) / OVERLORD_SUPPLY_PROVIDED;
        double ceiling = ceil(make_overlord_ct);
        make_overlord_ct = (ceiling <= 0.0 ? 1.0 : ceiling);
        return (int)make_overlord_ct;
    }
    return 0;
}