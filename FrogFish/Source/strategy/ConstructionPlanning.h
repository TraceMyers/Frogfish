#pragma once

#include "../production/BuildOrder.h"

using namespace Production;

namespace Strategy::ConstructionPlanning {

    BWAPI::TilePosition get_construction_location(const BuildOrder::Item& item);

}