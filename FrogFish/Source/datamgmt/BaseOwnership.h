#pragma once

#include <BWEM/bwem.h>
#include "../data/BaseStorage.h"
#include "../data/UnitStorage.h"

void init_base_storage(BWEM::Map &the_map, BaseStorage &base_storage);
void assign_new_bases(
    BWEM::Map &the_map, 
    BaseStorage &base_storage, 
    UnitStorage &unit_storage
);
void unassign_bases(BaseStorage &base_storage);

// internal
void self_assign_new_bases(
    BWEM::Map &the_map, 
    BaseStorage &base_storage, 
    const FUArray &self_units,
    const BWEMBArray &neutral_bases
); 
void enemy_assign_new_bases(
    BWEM::Map &the_map, 
    BaseStorage &base_storage, 
    const EUArray &enemy_units,
    const BWEMBArray &neutral_bases
);