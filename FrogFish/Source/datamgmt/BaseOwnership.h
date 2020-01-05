#pragma once

#include <BWEM/bwem.h>
#include "../data/BaseStorage.h"
#include "../data/UnitStorage.h"

void init_base_storage(BWEM::Map &the_map, BaseStorage &base_storage);
void self_assign_new_bases(
    BWEM::Map &the_map, 
    BaseStorage &base_storage, 
    UnitStorage &unit_storage
);

// internal
void self_assign_new_bases_iter(
    BWEM::Map &the_map, 
    BaseStorage &base_storage, 
    const FUArray &self_units,
    const BWEMBArray &neutral_bases
); 