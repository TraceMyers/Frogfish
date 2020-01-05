#pragma once

#include "FrogUnit.h"
#include "../utility/storage/FUArray.h"
#include <BWAPI.h>
#include <BWEM/bwem.h>
#include <vector>

class FrogBase {
// instantiate only once
// use free_data() when done

private:

    const BWEM::Base *bwem_base;
    FUArray workers;
    FUArray structures;
    FUnit resource_depot = NULL;

public:

    FrogBase(const BWEM::Base *_bwem_base) : bwem_base(_bwem_base) {}

    void add_worker(FUnit worker) {workers.add(worker);}

    void remove_worker(FUnit worker) {workers.remove(worker->get_ID());}

    void add_structure(FUnit structure) {structures.add(structure);}

    void remove_structure(FUnit structure) {structures.remove(structure->get_ID());}

    void assign_resource_depot(FUnit _resource_depot) {resource_depot = _resource_depot;}

    void unassign_resource_depot() {resource_depot = NULL;}

    int get_worker_ct() {return workers.length();}

    int get_structure_ct() {return structures.length();}

    bool has_resource_depot() {return resource_depot != NULL;}

    const FUArray &get_structures() {return structures;}

    const FUArray &get_workers() {return workers;}

    const FUnit get_resource_depot() {return resource_depot;}

    // only called when freeing this instance
    const BWEM::Base *free_data() {
        workers.free_data();
        structures.free_data();
        return bwem_base;
    }

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