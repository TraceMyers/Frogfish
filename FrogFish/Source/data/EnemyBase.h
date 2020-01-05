#pragma once

#include "EnemyUnit.h"
#include "../utility/storage/EUArray.h"
#include <BWAPI.h>
#include <BWEM/bwem.h>
#include <vector>

class EnemyBase {
// instantiate only once
// use free_data() when done

private:

    const BWEM::Base *bwem_base;
    EUArray workers;
    EUArray structures;
    EUnit resource_depot = NULL;

public:

    EnemyBase(const BWEM::Base *_bwem_base) : bwem_base(_bwem_base) {}

    void add_worker(EUnit worker) {workers.add(worker);}

    void remove_worker(EUnit worker) {workers.remove(worker->get_ID());}

    void add_structure(EUnit structure) {structures.add(structure);}

    void remove_structure(EUnit structure) {structures.remove(structure->get_ID());}

    void assign_resource_depot(EUnit _resource_depot) {resource_depot = _resource_depot;}

    void unassign_resource_depot() {resource_depot = NULL;}

    int get_worker_ct() {return workers.length();}

    int get_structure_ct() {return structures.length();}

    bool has_resource_depot() {return resource_depot != NULL;}

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

typedef class EnemyBase *EBase;