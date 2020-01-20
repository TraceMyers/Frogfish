#include "FrogUnit.h"
#pragma once
#pragma message("including TechStorage")

#include "BaseStorage.h"
#include <BWAPI.h>

// TODO: implement enemy as well

class TechStorage {

private:

    std::map<BWAPI::UnitType, bool> _self_can_make;
    std::vector<BWAPI::UpgradeType> self_upgrades;
    std::vector<BWAPI::TechType> self_techs;

public:

    void on_frame_update(BaseStorage &base_storage);
    bool self_can_make(BWAPI::UnitType &ut);
};