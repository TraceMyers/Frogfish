#pragma once

#include <BWEM/bwem.h>
#include "../data/UnitStorage.h"
#include "../data/BaseStorage.h"

void assign_base_assets(
    BWEM::Map &the_map, 
    BaseStorage &base_storage,
    UnitStorage &unit_storage
);
template <class UnitArrayT, class BaseArrayT>
void assign_assets(
    BWEM::Map &the_map, 
    BaseStorage &base_storage,
    const UnitArrayT &units,
    const BaseArrayT &bases
);
template <class UnitArrayT, class BaseArrayT, class UnitT>
void assign_structure(
    BWEM::Map &the_map, 
    BaseStorage &base_storage,
    const UnitArrayT &units,
    const BaseArrayT &bases,
    const UnitT &unit
);
void unassign_base_assets(
    BWEM::Map &the_map, 
    BaseStorage &base_storage, 
    UnitStorage &unit_storage
);
template <class BaseArrayT, class UnitArrayT>
void unassign_assets(const BaseArrayT &bases, const UnitArrayT &units);
template <class BaseArrayT, class UnitArrayT>
void unassign_assets_unconditional(
    const BaseArrayT &bases, 
    const UnitArrayT &units
);
template <class BaseArrayT>
void unassign_wrong_area_assets(BWEM::Map &the_map, const BaseArrayT &bases);
template <class BaseT, class UnitArrayT>
void unassign_wrong_area_structure(BWEM::Map &the_map, BaseT base, UnitArrayT group);
template <class BaseT, class UnitArrayT>
void unassign_wrong_area_worker(BWEM::Map &the_map, BaseT base, UnitArrayT group);