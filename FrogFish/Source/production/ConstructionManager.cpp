#include "ConstructionManager.h"
#include "BuildOrder.h"
#include "EconTracker.h"
#include "BuildPlacement.h"
#include "../unitdata/BaseStorage.h"
#include "../unitdata/UnitStorage.h"
#include "../unitdata/FrogUnit.h"
#include <BWAPI.h>

// for now, just follows the build order and just one instruction at a time
// to build anywhere with any unit at the supply of the build item

void ConstructionManager::take_build_order(BuildOrder *_build_order) {
    build_order = _build_order;
}
void ConstructionManager::build_structures(
    EconTracker &econ_tracker, 
    BaseStorage &base_storage,
    UnitStorage &unit_storage
) {
    
}