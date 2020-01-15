#pragma once
#pragma message("BuildOrder")

#include <BWAPI.h>
#include <fstream>
#include <thread>

namespace TypeAndName {
    const static int zerg_type_ct = 28;
    const static char *zerg_names[] {
        "drone",
        "zergling",
        "hydralisk",
        "lurker",
        "ultralisk",
        "defiler",
        "overlord",
        "mutalisk",
        "scourge",
        "queen",
        "guardian",
        "devourer",
        "hatchery",
        "creep_colony",
        "sunken_colony",
        "spore_colony",
        "extractor",
        "spawning_pool",
        "evolution_chamber",
        "hydralisk_den",
        "lair",
        "spire",
        "queens_nest",
        "hive",
        "greater_spire",
        "nydus_canal",
        "ultralisk_cavern",
        "defiler_mound"
    };

    const static BWAPI::UnitType zerg_types[] {
        BWAPI::UnitTypes::Zerg_Drone,
        BWAPI::UnitTypes::Zerg_Zergling,
        BWAPI::UnitTypes::Zerg_Hydralisk,
        BWAPI::UnitTypes::Zerg_Lurker,
        BWAPI::UnitTypes::Zerg_Ultralisk,
        BWAPI::UnitTypes::Zerg_Defiler,
        BWAPI::UnitTypes::Zerg_Overlord,
        BWAPI::UnitTypes::Zerg_Mutalisk,
        BWAPI::UnitTypes::Zerg_Scourge,
        BWAPI::UnitTypes::Zerg_Queen,
        BWAPI::UnitTypes::Zerg_Guardian,
        BWAPI::UnitTypes::Zerg_Devourer,
        BWAPI::UnitTypes::Zerg_Hatchery,
        BWAPI::UnitTypes::Zerg_Creep_Colony,
        BWAPI::UnitTypes::Zerg_Sunken_Colony,
        BWAPI::UnitTypes::Zerg_Spore_Colony,
        BWAPI::UnitTypes::Zerg_Extractor,
        BWAPI::UnitTypes::Zerg_Spawning_Pool,
        BWAPI::UnitTypes::Zerg_Evolution_Chamber,
        BWAPI::UnitTypes::Zerg_Hydralisk_Den,
        BWAPI::UnitTypes::Zerg_Lair,
        BWAPI::UnitTypes::Zerg_Spire,
        BWAPI::UnitTypes::Zerg_Queens_Nest,
        BWAPI::UnitTypes::Zerg_Hive,
        BWAPI::UnitTypes::Zerg_Greater_Spire,
        BWAPI::UnitTypes::Zerg_Nydus_Canal,
        BWAPI::UnitTypes::Zerg_Ultralisk_Cavern,
        BWAPI::UnitTypes::Zerg_Defiler_Mound
    };
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
        CANCEL
    } build_type;

    const BWAPI::UnitType make_type;
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
        int _supply_target,
        int _count,
        int _required_item_i
    ) :
        condition_type(_cond),
        build_type(_build),
        make_type(_make_type),
        supply_target(_supply_target),
        count(_count),
        required_i(_required_item_i)
    {}

};

class BuildOrder {

public:

    std::string race;
    std::string name;
    std::vector<BuildItem> build_items;
    int cur_item;

    void load(const char *_race, const char *build_name) {
        cur_item = 0;
        build_items.clear();
        race = _race;
        name = build_name;
        std::ifstream in_file;
        in_file.open("bwapi-data\\read\\BuildOrders.txt");
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
                            BWAPI::UnitType unit_type = BWAPI::UnitTypes::Unknown;
                            int supply_target;
                            int count;
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
                                for (int i = 0; i < TypeAndName::zerg_type_ct; ++i) {
                                    if (word == TypeAndName::zerg_names[i]) {
                                        unit_type = TypeAndName::zerg_types[i];
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

    void add_item(
        BuildItem::CONDITION_TYPE condition_type,
        BuildItem::BUILD_TYPE build_type,
        BWAPI::UnitType make_type,
        int supply_target,
        int count,
        int required_i
    ) {
        build_items.push_back(
            BuildItem(
                condition_type,
                build_type,
                make_type,
                supply_target,
                count,
                required_i
            )
        );
    }

    BuildItem &peek_next() {
        return build_items[cur_item];
    }

    BuildItem &next() {
        ++cur_item;
        printf("build order next item: %d\n", cur_item);
        return build_items[cur_item - 1];
    }

    BuildItem &get(int i) {
        return build_items[i];
    }

    bool finished() {
        return cur_item >= build_items.size();
    }

    void print() {
        printf("build order %s %s\n", race.c_str(), name.c_str());
        for (unsigned int j = 0; j < build_items.size(); ++j) {
            BuildItem &bi = build_items[j];
            printf(
                "%d: condition %d, buildtype %d, supply %d, unit type %s, points to %d\n",
                j,
                bi.condition_type,
                bi.build_type,
                bi.supply_target,
                bi.make_type.getName().c_str(),
                bi.required_i
            );
        }
    }
};