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

namespace Zerg {
    extern std::map<UnitType, UnitType> UNIT_REQ;
    extern const int TYPE_CT;
    extern const int TECH_CT;
    extern const int UPGRADE_CT;
    extern const char *NAMES[28];
    extern const BWAPI::UnitType TYPES[28];
    extern const char *TECH_NAMES[9];
    extern const BWAPI::TechType TECH_TYPES[9];
    extern const char *UPGRADE_NAMES[16];
    extern const BWAPI::UpgradeType UPGRADE_TYPES[16];
}

}