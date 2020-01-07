#pragma once

#include <BWEM/bwem.h>
#include "../data/UnitStorage.h"
#include "../data/BaseStorage.h"

void assign_base_assets(
    BWEM::Map &the_map, 
    BaseStorage &base_storage,
    UnitStorage &unit_storage
);
template <class UnitT, class BaseT>
void assign_assets(
    BWEM::Map &the_map, 
    BaseStorage &base_storage,
    const std::vector<UnitT> &units,
    const std::vector<BaseT> &bases
);
template <class UnitT, class BaseT>
void assign_structure(
    BWEM::Map &the_map, 
    BaseStorage &base_storage,
    const std::vector<UnitT> &units,
    const std::vector<BaseT> &bases,
    UnitT unit
);
void unassign_base_assets(
    BWEM::Map &the_map, 
    BaseStorage &base_storage, 
    UnitStorage &unit_storage
);
template <class UnitT, class BaseT>
void unassign_assets(
    const std::vector<UnitT> &units, 
    const std::vector<BaseT> &bases
);
template <class UnitT, class BaseT>
void unassign_assets_unconditional(
    const std::vector<UnitT> &units, 
    const std::vector<BaseT> &bases
);
template <class BaseT>
void unassign_wrong_area_assets(BWEM::Map &the_map, const std::vector<BaseT> &bases);
template <class UnitT, class BaseT>
void unassign_wrong_area_asset(
    BWEM::Map &the_map, 
    std::vector<UnitT> group,
    BaseT base  
);