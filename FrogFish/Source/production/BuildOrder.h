#pragma once
#pragma message("including BuildOrder")

#include <BWAPI.h>
#include <fstream>
#include <thread>

namespace TypeAndName {
    extern const int ZERG_TYPE_CT;
    extern const int ZERG_TECH_CT;
    extern const int ZERG_UPGRADE_CT;
    extern const char *ZERG_NAMES[28];
    extern const BWAPI::UnitType ZERG_TYPES[28];
    extern const char *ZERG_TECH_NAMES[9];
    extern const BWAPI::TechType ZERG_TECH_TYPES[9];
    extern const char *ZERG_UPGRADE_NAMES[16];
    extern const BWAPI::UpgradeType ZERG_UPGRADE_TYPES[16];
}

class BuildItem {

public:

    const enum CONDITION_TYPE {
        SUPPLY,
        FINISHED
    } condition_type;

    const enum BUILD_TYPE {
        MAKE_UNIT,
        MORPH_UNIT,
        BUILD,
        TECH,
        UPGRADE,
        CANCEL
    } build_type;

    const BWAPI::UnitType make_type;
    const BWAPI::TechType tech_type;
    const BWAPI::UpgradeType upgrade_type;
    const int count;
    const int supply_target;
    // in an array of order items, required_item_i points back to the
    // index of the item this item requires...
    // i.e. : if it requires a building to be finished
    const int required_i;

    BuildItem(
        CONDITION_TYPE _cond,
        BUILD_TYPE _build,
        BWAPI::UnitType _make_type,
        BWAPI::TechType _tech_type,
        BWAPI::UpgradeType _unit_type,
        int _supply_target,
        int _count,
        int _required_item_i
    );
};

class BuildOrder {

public:

    std::string race;
    std::string name;
    std::vector<BuildItem> build_items;
    int cur_item;

    void load(const char *_race, const char *build_name);
    void add_item(
        BuildItem::CONDITION_TYPE condition_type,
        BuildItem::BUILD_TYPE build_type,
        BWAPI::UnitType make_type,
        BWAPI::TechType tech_type,
        BWAPI::UpgradeType upgrade_type,
        int supply_target,
        int count,
        int required_i
    );
    BuildItem &peek_next();
    BuildItem &next();
    BuildItem &get(int i);
    unsigned size();
    bool finished();
    void print();
};