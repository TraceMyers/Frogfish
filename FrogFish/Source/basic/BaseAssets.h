#pragma once
#pragma message("including BaseAssets")

#include "../unitdata/UnitStorage.h"
#include "../unitdata/BaseStorage.h"

namespace BaseAssets {

    void assign_base_assets(BaseStorage &base_storage, UnitStorage &unit_storage);

}
