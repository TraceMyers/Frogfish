#pragma once

#include <BWEM/bwem.h>
#include "../data/UnitStorage.h"
#include "../data/BaseStorage.h"

void assign_structures_to_bases(
    BWEM::Map &the_map, 
    BaseStorage &base_storage,
    UnitStorage &unit_storage
);
void unassign_base_structures(
        BWEM::Map &the_map, 
        BaseStorage &base_storage, 
        UnitStorage &unit_storage
);

// internal
void self_assign_structures(
    BWEM::Map &the_map, 
    BaseStorage &base_storage,
    const FUArray &self_units,
    const FBArray &self_bases
);
void enemy_assign_structures(
    BWEM::Map &the_map, 
    BaseStorage &base_storage,
    const EUArray &enemy_units,
    const EBArray &enemy_bases
);
void enemy_assign_structures_zerg_morph(
    BWEM::Map &the_map, 
    BaseStorage &base_storage,
    const EUArray &enemy_units,
    const EBArray &enemy_bases
);
void unassign_self_structures(const FBArray &self_bases, const FUArray &self_units);
void unassign_self_structures_conditionless(
    const FBArray &self_bases, 
    const FUArray &self_units
);
void unassign_enemy_structures_zerg(const EBArray &enemy_bases, const EUArray &enemy_units);
void unassign_enemy_structures_terran(BWEM::Map &the_map, const EBArray &enemy_bases);
void unassign_enemy_structures_conditionless(
    const EBArray &enemy_bases, 
    const EUArray &enemy_units
);