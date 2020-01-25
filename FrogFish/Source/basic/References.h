#pragma once
#pragma message("including References")

#include <BWAPI.h>
#include <map>

using namespace BWAPI;
using namespace BWAPI::UnitTypes;

namespace Basic::Refs {

enum UTASK {
    IDLE,
    MINERALS,
    GAS,
    TRANSFER,
    BUILD,
    MORPH,
    ATTACK,
    SCOUT
};

enum UTYPE {
    UNASSIGNED,
    EGG,
    LARVA,
    WORKER,
    ARMY,
    STRUCT
};

namespace ZergReqs {
    extern std::map<UnitType, UnitType> ZERG_UNIT_REQ;
}

}