#pragma once

#include <BWEM/bwem.h>
#include "../utility/BWEMBaseArray.h"
#include "../data/BaseStorage.h"
#include "../data/UnitStorage.h"

void update_base_data(
    BWEM::Map &the_map, 
    BaseStorage &base_storage, 
    UnitStorage &unit_storage
);
void init_base_storage(BWEM::Map &the_map, BaseStorage &base_storage);
void assign_new_bases(
    BWEM::Map &the_map, 
    BaseStorage &base_storage, 
    UnitStorage &unit_storage
);
template <class UnitArrayT>
void self_assign_new_bases(
    BWEM::Map &the_map, 
    BaseStorage &base_storage, 
    const UnitArrayT &self_units,
    const BWEMBaseArray &neutral_bases
);
template <class UnitArrayT>
void enemy_assign_new_bases(
    BWEM::Map &the_map, 
    BaseStorage &base_storage, 
    const UnitArrayT &enemy_units,
    const BWEMBaseArray &neutral_bases
);
void unassign_bases(BaseStorage &base_storage);
