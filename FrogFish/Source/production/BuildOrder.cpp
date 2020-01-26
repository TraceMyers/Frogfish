#include "BuildOrder.h"
#include <BWAPI.h>
#include <fstream>
#include <thread>



BuildItem::BuildItem(
    CONDITION_TYPE _cond,
    BUILD_TYPE _build,
    BWAPI::UnitType _make_type,
    BWAPI::TechType _tech_type,
    BWAPI::UpgradeType _upgrade_type,
    int _supply_target,
    int _count,
    int _required_item_i
) :
    condition_type(_cond),
    build_type(_build),
    make_type(_make_type),
    tech_type(_tech_type),
    upgrade_type(_upgrade_type),
    supply_target(_supply_target),
    count(_count),
    required_i(_required_item_i)
{}


void BuildOrder::load(const char *_race, const char *build_name) {
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
                        BuildItem::CONDITION_TYPE cond_type;
                        BuildItem::BUILD_TYPE build_type;
                        BWAPI::UnitType unit_type = BWAPI::UnitTypes::None;
                        BWAPI::TechType tech_type = BWAPI::TechTypes::None;
                        BWAPI::UpgradeType upgrade_type = BWAPI::UpgradeTypes::None;
                        int supply_target = 0;
                        int count = 0;
                        int required_i = -1;

                        in_file >> word;
                        word.pop_back();
                        if (word == "supply") {
                            cond_type = BuildItem::SUPPLY;
                        } 
                        else if (word == "finished") {
                            cond_type = BuildItem::FINISHED;
                        }
                        else {
                            printf("BuildOrder error: cond_type = %s\n", word.c_str());
                        }

                        in_file >> word;
                        word.pop_back();
                        supply_target = std::stoi(word) * 2;

                        in_file >> word;
                        word.pop_back();
                        if (word == "build") {
                            build_type = BuildItem::BUILD;
                        }
                        else if (word == "make") {
                            build_type = BuildItem::MAKE_UNIT;
                        }
                        else if (word == "tech") {
                            build_type = BuildItem::TECH;
                        }
                        else if (word == "upgrade") {
                            build_type = BuildItem::UPGRADE;
                        }
                        else if (word == "cancel") {
                            build_type = BuildItem::CANCEL;
                        }
                        else {
                            printf("BuildOrder error: build_type = %s\n", word.c_str());
                        }

                        in_file >> word;
                        word.pop_back();
                        count = std::stoi(word);

                        in_file >> word;
                        word.pop_back();
                        if (word != "null") {
                            if (
                                build_type == BuildItem::BUILD
                                || build_type == BuildItem::MAKE_UNIT
                                || build_type == BuildItem::CANCEL
                            ) {
                                for (int i = 0; i < TypeAndName::ZERG_TYPE_CT; ++i) {
                                    if (word == TypeAndName::ZERG_NAMES[i]) {
                                        unit_type = TypeAndName::ZERG_TYPES[i];
                                    }
                                }
                            }
                            else if (build_type == BuildItem::TECH) {
                                for (int i = 0; i < TypeAndName::ZERG_TECH_CT; ++i) {
                                    if (word == TypeAndName::ZERG_TECH_NAMES[i]) {
                                        tech_type = TypeAndName::ZERG_TECH_TYPES[i];
                                    }
                                }
                            }
                            else if (build_type == BuildItem::UPGRADE) {
                                for (int i = 0; i < TypeAndName::ZERG_UPGRADE_CT; ++i) {
                                    if (word == TypeAndName::ZERG_UPGRADE_NAMES[i]) {
                                        upgrade_type = TypeAndName::ZERG_UPGRADE_TYPES[i];
                                    }
                                }
                            }
                        }

                        in_file >> word;
                        if (word != "null") {
                            required_i = std::stoi(word);
                        }

                        add_item(
                            cond_type, 
                            build_type, 
                            unit_type,
                            tech_type,
                            upgrade_type,
                            supply_target, 
                            count, 
                            required_i
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

void BuildOrder::add_item(
    BuildItem::CONDITION_TYPE condition_type,
    BuildItem::BUILD_TYPE build_type,
    BWAPI::UnitType make_type,
    BWAPI::TechType tech_type,
    BWAPI::UpgradeType upgrade_type,
    int supply_target,
    int count,
    int required_i
) {
    build_items.push_back(
        BuildItem(
            condition_type,
            build_type,
            make_type,
            tech_type,
            upgrade_type,
            supply_target,
            count,
            required_i
        )
    );
}

BuildItem &BuildOrder::peek_next() {
    return build_items[cur_item];
}

BuildItem &BuildOrder::next() {
    ++cur_item;
    printf("build order next item: %d\n", cur_item);
    return build_items[cur_item - 1];
}

BuildItem &BuildOrder::get(int i) {
    return build_items[i];
}

unsigned BuildOrder::size() {
    return build_items.size();
}

bool BuildOrder::finished() {
    return (unsigned int)cur_item >= build_items.size();
}

void BuildOrder::print() {
    printf("build order %s %s\n", race.c_str(), name.c_str());
    for (unsigned int j = 0; j < build_items.size(); ++j) {
        BuildItem &bi = build_items[j];
        printf(
            "%d: condition %d, buildtype %d, supply %d, unit type %s, points to %d\n",
            (int)j,
            bi.condition_type,
            bi.build_type,
            bi.supply_target,
            bi.make_type.getName().c_str(),
            bi.required_i
        );
    }
}