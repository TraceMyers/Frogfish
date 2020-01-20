#pragma once
#pragma message("including Requirements")

#include <BWAPI.h>
#include <map>

namespace ZergRequirements {

    using namespace BWAPI;
    using namespace BWAPI::UnitTypes;

    extern std::map<UnitType, UnitType> ZERG_UNIT_REQ;

}