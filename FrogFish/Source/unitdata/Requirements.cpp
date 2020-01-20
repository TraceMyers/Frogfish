#include "Requirements.h"
#include <BWAPI.h>
#include <map>

namespace ZergRequirements {

    using namespace BWAPI;
    using namespace BWAPI::UnitTypes;

    std::map<UnitType, UnitType> ZERG_UNIT_REQ = {
        {Zerg_Drone, None},
        {Zerg_Zergling, Zerg_Spawning_Pool},
        {Zerg_Hydralisk, Zerg_Hydralisk_Den},
        {Zerg_Lurker, Zerg_Hydralisk_Den},
        {Zerg_Ultralisk, Zerg_Ultralisk_Cavern},
        {Zerg_Defiler, Zerg_Defiler_Mound},
        {Zerg_Overlord, None},
        {Zerg_Mutalisk, Zerg_Spire},
        {Zerg_Scourge, Zerg_Spire},
        {Zerg_Queen, Zerg_Queens_Nest},
        {Zerg_Guardian, Zerg_Greater_Spire},
        {Zerg_Devourer, Zerg_Greater_Spire},
        {Zerg_Hatchery, None},
        {Zerg_Creep_Colony, None},
        {Zerg_Sunken_Colony, Zerg_Spawning_Pool},
        {Zerg_Spore_Colony, Zerg_Spawning_Pool},
        {Zerg_Extractor, None},
        {Zerg_Spawning_Pool, None},
        {Zerg_Evolution_Chamber, None},
        {Zerg_Hydralisk_Den, Zerg_Spawning_Pool},
        {Zerg_Lair, Zerg_Hatchery},
        {Zerg_Spire, Zerg_Lair},
        {Zerg_Queens_Nest, Zerg_Lair},
        {Zerg_Hive, Zerg_Queens_Nest},
        {Zerg_Greater_Spire, Zerg_Hive},
        {Zerg_Nydus_Canal, Zerg_Hive},
        {Zerg_Ultralisk_Cavern, Zerg_Hive},
        {Zerg_Defiler_Mound, Zerg_Hive}
    };
}