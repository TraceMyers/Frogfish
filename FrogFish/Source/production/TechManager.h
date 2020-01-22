#pragma once
#pragma message("including TechManager")

#include "../unitdata/BaseStorage.h"
#include "BuildOrder.h"
#include "EconTracker.h"
#include <BWAPI.h>
#include <vector>


class TechManager {

private:

    FUnit sent_research_command;
    BWAPI::UpgradeType sent_upgrade;
    BWAPI::TechType sent_tech;

public:

    void init_upgrades(
        BaseStorage &base_storage, 
        BuildOrder *build_order, 
        EconTracker &econ_tracker
    );
};
