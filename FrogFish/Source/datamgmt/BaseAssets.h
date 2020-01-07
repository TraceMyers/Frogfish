#pragma once

#include <BWEM/bwem.h>
#include "../data/UnitStorage.h"
#include "../data/BaseStorage.h"

void assign_base_assets(
    BWEM::Map &the_map, 
    BaseStorage &base_storage,
    UnitStorage &unit_storage
);
