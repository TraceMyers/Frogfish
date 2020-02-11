#include "BuildOrder.h"
#include "../basic/References.h"
#include <BWAPI.h>
#include <fstream>
#include <thread>

// TODO: only extra functionality needed is morph-transitioning between build orders
// TODO: instead of UnitMaker doing proportional making, anything that is made goes
// into the build order, so those proportional calcs are done, units placed here,
// and unitmaker makes the same way the whole game - from the build order
using namespace Basic;

namespace Production::BuildOrder { 

Item::Item(
    ACTION _action,
    BWAPI::UnitType _unit_type,
    BWAPI::TechType _tech_type,
    BWAPI::UpgradeType _upgrade_type,
    int _count,
    int _cancel_index
) :
    action(_action),
    unit_type(_unit_type),
    tech_type(_tech_type),
    upgrade_type(_upgrade_type),
    count(_count),
    cancel_index(_cancel_index)
{}


void load(const char *_race, const char *build_name) {
    cur_item = 0;
    build_items.clear();
    race = _race;
    name = build_name;
    std::ifstream in_file;
    in_file.open("C:\\ProgramData\\bwapi\\starcraft\\bwapi-data\\read\\BuildOrders.txt");
    if (!in_file) {
        printf("not able to read build order file\n");
    }
    std::string word;
    bool loaded = false;
    while (!loaded && in_file >> word) {
        if (word == race) {
            in_file >> word;
            if (word == build_name) {
                while(in_file >> word) {
                    if (word[0] != '-') {
                        Item::ACTION action;
                        BWAPI::UnitType unit_type = BWAPI::UnitTypes::None;
                        BWAPI::TechType tech_type = BWAPI::TechTypes::None;
                        BWAPI::UpgradeType upgrade_type = BWAPI::UpgradeTypes::None;
                        int count = 0;
                        int cancel_index = -1;

                        in_file >> word;
                        word.pop_back();
                        if (word == "build") {
                            action = Item::BUILD;
                        }
                        else if (word == "make") {
                            action = Item::MAKE_UNIT;
                        }
                        else if (word == "tech") {
                            action = Item::TECH;
                        }
                        else if (word == "upgrade") {
                            action = Item::UPGRADE;
                        }
                        else if (word == "cancel") {
                            action = Item::CANCEL;
                        }
                        else {
                            printf("BuildOrder error: action = %s\n", word.c_str());
                        }

                        in_file >> word;
                        word.pop_back();
                        count = std::stoi(word);

                        in_file >> word;
                        word.pop_back();
                        if (word != "null") {
                            if (
                                action == Item::BUILD
                                || action == Item::MAKE_UNIT
                                || action == Item::CANCEL
                            ) {
                                for (int i = 0; i < Refs::Zerg::TYPE_CT; ++i) {
                                    if (word == Refs::Zerg::NAMES[i]) {
                                        unit_type = Refs::Zerg::TYPES[i];
                                    }
                                }
                            }
                            else if (action == Item::TECH) {
                                for (int i = 0; i < Refs::Zerg::TECH_CT; ++i) {
                                    if (word == Refs::Zerg::TECH_NAMES[i]) {
                                        tech_type = Refs::Zerg::TECH_TYPES[i];
                                    }
                                }
                            }
                            else if (action == Item::UPGRADE) {
                                for (int i = 0; i < Refs::Zerg::UPGRADE_CT; ++i) {
                                    if (word == Refs::Zerg::UPGRADE_NAMES[i]) {
                                        upgrade_type = Refs::Zerg::UPGRADE_TYPES[i];
                                    }
                                }
                            }
                        }

                        in_file >> word;
                        if (word != "null") {
                            cancel_index = std::stoi(word);
                        }

                        add_item (
                            action,
                            unit_type,
                            tech_type,
                            upgrade_type,
                            count,
                            cancel_index
                        );
                    }
                    else {
                        loaded = true;
                        break;
                    }
                }
            }
        }
    }
    in_file.close();
}

void add_item(
    Item::ACTION action,
    BWAPI::UnitType unit_type,
    BWAPI::TechType tech_type,
    BWAPI::UpgradeType upgrade_type,
    int count,
    int cancel_index
) {
    build_items.push_back(
        Item(
            action,
            unit_type,
            tech_type,
            upgrade_type,
            count,
            cancel_index
        )
    );
}

Item &peek_next() {
    return build_items[cur_item];
}

Item &next() {
    ++cur_item;
    printf("build order next item: %d\n", cur_item);
    return build_items[cur_item - 1];
}

Item &get(int i) {
    return build_items[i];
}

unsigned size() {
    return build_items.size();
}

bool finished() {
    return (unsigned int)cur_item >= build_items.size();
}

}