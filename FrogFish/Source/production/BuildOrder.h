#pragma once
#pragma message("including BuildOrder")

#include <BWAPI.h>
#include <fstream>
#include <thread>

namespace Production::BuildOrder {
    
struct Item {
    const enum ACTION {
        MAKE_UNIT,
        MORPH_UNIT,
        BUILD,
        TECH,
        UPGRADE,
        CANCEL
    } action;

    const BWAPI::UnitType unit_type;
    const BWAPI::TechType tech_type;
    const BWAPI::UpgradeType upgrade_type;
    const int count;
    const int cancel_index;

    Item(
        ACTION action,
        BWAPI::UnitType _unit_type,
        BWAPI::TechType _tech_type,
        BWAPI::UpgradeType _upgrade_type,
        int _count,
        int _cancel_index
    );
};

std::string       race;
std::string       name;
std::vector<Item> build_items;
int               cur_item;

void     load(const char *_race, const char *build_name);
void     add_item(
    Item::ACTION action,
    BWAPI::UnitType unit_type,
    BWAPI::TechType tech_type,
    BWAPI::UpgradeType upgrade_type,
    int count,
    int cancel_i
);
Item &   peek_next();
Item &   next();
Item &   get(int i);
unsigned size();
bool     finished();
void     print();

}