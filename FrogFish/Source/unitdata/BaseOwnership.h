#pragma once

#include <BWEM/bwem.h>
#include "../unitdata/BWEMBaseArray.h"
#include "../unitdata/BaseStorage.h"
#include "../unitdata/UnitStorage.h"

void update_base_data(BaseStorage &base_storage, UnitStorage &unit_storage);
void init_base_storage(BaseStorage &base_storage);
void assign_new_bases(BaseStorage &base_storage, UnitStorage &unit_storage);
template <class UnitArrayT>
void self_assign_new_bases(
    BaseStorage &base_storage, 
    const UnitArrayT &self_units,
    const BWEMBaseArray &neutral_bases
);
template <class UnitArrayT>
void enemy_assign_new_bases(
    BaseStorage &base_storage, 
    const UnitArrayT &enemy_units,
    const BWEMBaseArray &neutral_bases
);
void unassign_bases(BaseStorage &base_storage);
