#pragma once

#include "FrogUnit.h"
#include <BWAPI.h>
#include <BWEM/bwem.h>
#include <vector>

class FrogBase {
// instantiate only once
// use free_data() when done

private:

    const BWEM::Base *bwem_base;
    std::vector<FUnit> workers;
    std::vector<FUnit> structures;
    std::vector<FUnit> resource_depots;

public:

    FrogBase(const BWEM::Base *_bwem_base) : bwem_base(_bwem_base) {}

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

    int get_worker_ct() {return workers.size();}

    int get_structure_ct() {return structures.size();}

    bool has_resource_depot() {return resource_depots.size() > 0;}

    const std::vector<FUnit> &get_structures() {return structures;}

    const std::vector<FUnit> &get_workers() {return workers;}

    const std::vector<FUnit> &get_resource_depot() {return resource_depots;}

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