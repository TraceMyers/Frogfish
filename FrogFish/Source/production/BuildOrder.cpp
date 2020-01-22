#include "BuildOrder.h"
#include <BWAPI.h>
#include <fstream>
#include <thread>

const int TypeAndName::ZERG_TYPE_CT = 28;

const int TypeAndName::ZERG_TECH_CT = 9;

const int TypeAndName::ZERG_UPGRADE_CT = 16;

const char *TypeAndName::ZERG_NAMES[28] = {
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

const BWAPI::UnitType TypeAndName::ZERG_TYPES[28] = {
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

const char* TypeAndName::ZERG_TECH_NAMES[9] = {
    "lurker_aspect",
    "parasite",
    "plague",
    "burrow",
    "consume",
    "ensnare",
    "infestation",
    "dark_swarm",
    "spawn_broodlings"

};

const BWAPI::TechType TypeAndName::ZERG_TECH_TYPES[9] = {
    BWAPI::TechTypes::Lurker_Aspect,
    BWAPI::TechTypes::Parasite,
    BWAPI::TechTypes::Plague,
    BWAPI::TechTypes::Burrowing,
    BWAPI::TechTypes::Consume,
    BWAPI::TechTypes::Ensnare,
    BWAPI::TechTypes::Infestation,
    BWAPI::TechTypes::Dark_Swarm,
    BWAPI::TechTypes::Spawn_Broodlings
};

const char* TypeAndName::ZERG_UPGRADE_NAMES[16] = {
    "adrenal_glands",
    "anabolic_synthesis",
    "antennae",
    "chitinous_plating",
    "gamete_meiosis",
    "grooved_spines",
    "metabolic_boost",
    "metasynaptic_node",
    "muscular_augments",
    "pneumatized_carapace",
    "ventral_sacs",
    "zerg_carapace",
    "zerg_flyer_attacks",
    "zerg_flyer_carapace",
    "zerg_melee_attacks",
    "zerg_missile_attacks"
};

const BWAPI::UpgradeType TypeAndName::ZERG_UPGRADE_TYPES[16] = {
    BWAPI::UpgradeTypes::Adrenal_Glands,
    BWAPI::UpgradeTypes::Anabolic_Synthesis,
    BWAPI::UpgradeTypes::Antennae,
    BWAPI::UpgradeTypes::Chitinous_Plating,
    BWAPI::UpgradeTypes::Gamete_Meiosis,
    BWAPI::UpgradeTypes::Grooved_Spines,
    BWAPI::UpgradeTypes::Metabolic_Boost,
    BWAPI::UpgradeTypes::Metasynaptic_Node,
    BWAPI::UpgradeTypes::Muscular_Augments,
    BWAPI::UpgradeTypes::Pneumatized_Carapace,
    BWAPI::UpgradeTypes::Ventral_Sacs,
    BWAPI::UpgradeTypes::Zerg_Carapace,
    BWAPI::UpgradeTypes::Zerg_Flyer_Attacks,
    BWAPI::UpgradeTypes::Zerg_Flyer_Carapace,
    BWAPI::UpgradeTypes::Zerg_Melee_Attacks,
    BWAPI::UpgradeTypes::Zerg_Missile_Attacks
};

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