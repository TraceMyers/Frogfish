#pragma once

#include "FrogUnit.h"
#include <BWAPI.h>
#include <BWEM/bwem.h>
#include <vector>

class BuildGraph;

class FrogBase {
// instantiate only once
// use free_data() when done

private:

    const BWEM::Base *bwem_base;
    std::vector<FUnit> 
        larva,
        workers,
        structures,
        resource_depots;


public:

    // for deciding what can/should be done at this base
    // i.e. if danger level red, don't make drones here, and secure
    // the base before transferring
    // yellow might be a base that is near the enemy, but not
    // currently under attack
    enum DANGER_LEVEL {
        VERY_LOW,
        LOW,
        MEDIUM,
        HIGH,
        VERY_HIGH
    };

    DANGER_LEVEL danger_level;

    FrogBase(const BWEM::Base *_bwem_base) : 
        bwem_base(_bwem_base), 
        danger_level(VERY_LOW)
    {}

    void add_larva(FUnit _larva) {larva.push_back(_larva);}

    void remove_larva(FUnit _larva) {
        std::vector<FUnit>::iterator it = std::remove(larva.begin(), larva.end(), _larva);
        larva.erase(it, larva.end());
    }

    void add_worker(FUnit worker) {workers.push_back(worker);}

    void remove_worker(FUnit worker) {
        std::vector<FUnit>::iterator it = std::remove(workers.begin(), workers.end(), worker);
        workers.erase(it, workers.end());
    }

    void add_structure(FUnit structure) {structures.push_back(structure);}

    void remove_structure(FUnit structure) {
        std::vector<FUnit>::iterator it = std::remove(structures.begin(), structures.end(), structure);
        structures.erase(it, structures.end());
    }

    void add_resource_depot(FUnit _resource_depot) {resource_depots.push_back(_resource_depot);}

    void remove_resource_depot(FUnit _resource_depot) {
        std::vector<FUnit>::iterator it = std::remove(resource_depots.begin(), resource_depots.end(), _resource_depot);
        resource_depots.erase(it, resource_depots.end());
    }

    int get_larva_ct() {return larva.size();}

    int get_worker_ct() {return workers.size();}

    int get_structure_ct() {return structures.size();}

    int get_resource_depot_ct() {return resource_depots.size();}

    bool has_larva(FUnit _larva) {
        std::vector<FUnit>::iterator it = std::find(larva.begin(), larva.end(), _larva);
        if (it != larva.end()) {
            return true;
        }
        return false;
    }

    bool has_worker(FUnit worker) {
        std::vector<FUnit>::iterator it = std::find(workers.begin(), workers.end(), worker);
        if (it != workers.end()) {
            return true;
        }
        return false;
    }

    bool has_structure(FUnit structure) {
        std::vector<FUnit>::iterator it = std::find(structures.begin(), structures.end(), structure);
        if (it != structures.end()) {
            return true;
        }
        return false;
    }

    void set_danger_level(DANGER_LEVEL level) {
        danger_level = level;
    }

    DANGER_LEVEL get_danger_level() {
        return danger_level;
    }

    const std::vector<FUnit> &get_larva() {return larva;}

    const std::vector<FUnit> &get_workers() {return workers;}

    const std::vector<FUnit> &get_structures() {return structures;}

    const std::vector<FUnit> &get_resource_depots() {return resource_depots;}

    // only called when freeing this instance
    const BWEM::Base *free_data() {return bwem_base;}

    // BWEM

    bool is_starting() {return bwem_base->Starting();}

    const BWEM::Area *get_area() {return bwem_base->GetArea();}

    const BWAPI::TilePosition &get_location() {return bwem_base->Location();}

    const BWAPI::Position &get_center() {return bwem_base->Center();}

    const std::vector<BWEM::Mineral *> &get_minerals() {return bwem_base->Minerals();}

    const std::vector<BWEM::Geyser *> &get_geysers() {return bwem_base->Geysers();}

    const std::vector<BWEM::Mineral *> &get_blocking_minerals() {return bwem_base->BlockingMinerals();}

};

typedef class FrogBase *FBase;