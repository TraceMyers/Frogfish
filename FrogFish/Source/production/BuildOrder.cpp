#include "BuildOrder.h"
#include "../basic/References.h"
#include <BWAPI.h>
#include <fstream>
#include <thread>
#include <cassert>

// TODO: only extra functionality needed is morph-transitioning between build orders
// TODO: instead of UnitMaker doing proportional making, anything that is made goes
// into the build order, so those proportional calcs are done, units placed here,
// and unitmaker makes the same way the whole game - from the build order

// Currently: transitioning from using Item to using InternalItem

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

    namespace {

        struct InternalItem : public Item {

            InternalItem(
                ACTION _action,
                BWAPI::UnitType _unit_type,
                BWAPI::TechType _tech_type,
                BWAPI::UpgradeType _upgrade_type,
                int _count,
                int _cancel_index
            ) : 
                Item (_action, _unit_type, _tech_type, _upgrade_type, _count, _cancel_index)
            {
                set_mineral_cost();
                set_gas_cost();
                set_supply_cost();
                set_larva_cost();
            }

            void set_mineral_cost() {
                switch(action) {
                    case Item::ACTION::TECH:
                        _mineral_cost = tech_type.mineralPrice();
                        break;
                    case Item::ACTION::UPGRADE:
                        _mineral_cost = upgrade_type.mineralPrice();
                        break;
                    case Item::ACTION::CANCEL:
                        if (unit_type != BWAPI::UnitTypes::None) {
                            if (unit_type.isBuilding()) {
                                _mineral_cost = (int)(-unit_type.mineralPrice() * 0.75);
                            }
                            else {
                                _mineral_cost = -unit_type.mineralPrice();
                            }
                        }
                        else if (tech_type != BWAPI::TechTypes::None) {
                            _mineral_cost = -tech_type.mineralPrice();
                        }
                        else {
                            _mineral_cost = -upgrade_type.mineralPrice();
                        }
                        break;
                    default:
                        _mineral_cost = unit_type.mineralPrice();
                }
            }

            void set_gas_cost() {
                switch(action) {
                    case Item::ACTION::TECH:
                        _gas_cost = tech_type.gasPrice();
                        break;
                    case Item::ACTION::UPGRADE:
                        _gas_cost = upgrade_type.gasPrice();
                        break;
                    case Item::ACTION::CANCEL:
                        if (unit_type != BWAPI::UnitTypes::None) {
                            if (unit_type.isBuilding()) {
                                _gas_cost = (int)(-unit_type.gasPrice() * 0.75);
                            }
                            else {
                                _gas_cost = -unit_type.gasPrice();
                            }
                        }
                        else if (tech_type != BWAPI::TechTypes::None) {
                            _gas_cost = -tech_type.gasPrice();
                        }
                        else {
                            _gas_cost = -upgrade_type.gasPrice();
                        }
                        break;
                    default:
                        _gas_cost = unit_type.gasPrice();
                }
            }

            void set_supply_cost() {
                switch(action) {
                    case Item::ACTION::BUILD:
                        _supply_cost = -2 - unit_type.supplyProvided();
                        break;
                    case Item::ACTION::MORPH:
                        _supply_cost = 
                            unit_type.supplyRequired() - unit_type.whatBuilds().first.supplyRequired();
                        break;
                    case Item::ACTION::MAKE:
                        _supply_cost = unit_type.supplyRequired() - unit_type.supplyProvided();
                        break;
                    case Item::ACTION::CANCEL:
                        _supply_cost = unit_type.supplyProvided() - unit_type.supplyRequired()
                            + unit_type.whatBuilds().first.supplyRequired();
                }
            }

            void set_larva_cost() {
                switch(action) {
                    case Item::ACTION::MAKE:
                        _larva_cost = 1;
                        break;
                    default:
                        _larva_cost = 0;
                }
            }
        };
    }

    namespace {
        std::string       race;
        std::string       name;
        std::vector<InternalItem> items;
        int               cur_index;

        void _push(InternalItem item) {
            items.push_back(item);
        }

        void _insert(InternalItem item, int i) {
            items.insert(items.begin() + i, item);
        }

        void _insert_next(InternalItem item) {
            items.insert(items.begin() + cur_index, item);
        }
    }

    void load(const char *_race, const char *build_name) {
        cur_index = 0;
        items.clear();
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
                                action = Item::MAKE;
                            }
                            else if (word == "morph") {
                                action = Item::MORPH;
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
                            if (
                                action == Item::BUILD
                                || action == Item::MAKE
                                || action == Item::MORPH
                                || action == Item::CANCEL
                            ) {
                                for (int i = 0; i < Refs::Zerg::TYPE_CT; ++i) {
                                    if (word == Refs::Zerg::NAMES[i]) {
                                        unit_type = Refs::Zerg::TYPES[i];
                                    }
                                }
                            }
                            else if (action == Item::TECH || action == Item::CANCEL) {
                                for (int i = 0; i < Refs::Zerg::TECH_CT; ++i) {
                                    if (word == Refs::Zerg::TECH_NAMES[i]) {
                                        tech_type = Refs::Zerg::TECH_TYPES[i];
                                    }
                                }
                            }
                            else if (action == Item::UPGRADE || action == Item::CANCEL) {
                                for (int i = 0; i < Refs::Zerg::UPGRADE_CT; ++i) {
                                    if (word == Refs::Zerg::UPGRADE_NAMES[i]) {
                                        upgrade_type = Refs::Zerg::UPGRADE_TYPES[i];
                                    }
                                }
                            }

                            in_file >> word;
                            if (word != "null") {
                                cancel_index = std::stoi(word);
                            }

                            InternalItem item(
                                action,
                                unit_type,
                                tech_type,
                                upgrade_type,
                                count,
                                cancel_index
                            );
                            _push(item);
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

    void push(
        Item::ACTION _action,
        BWAPI::UnitType _unit_type,
        BWAPI::TechType _tech_type,
        BWAPI::UpgradeType _upgrade_type,
        int _count,
        int _cancel_index
    ) {
        InternalItem item(
            _action,
            _unit_type,
            _tech_type,
            _upgrade_type,
            _count,
            _cancel_index
        );
        _push(item);
    }

    void insert(
        Item::ACTION _action,
        BWAPI::UnitType _unit_type,
        BWAPI::TechType _tech_type,
        BWAPI::UpgradeType _upgrade_type,
        int _count,
        int _cancel_index,
        int insert_index
    ) {
        InternalItem item(
            _action,
            _unit_type,
            _tech_type,
            _upgrade_type,
            _count,
            _cancel_index
        );
        _insert(item, insert_index);
    }

    void insert_next(
        Item::ACTION _action,
        BWAPI::UnitType _unit_type,
        BWAPI::TechType _tech_type,
        BWAPI::UpgradeType _upgrade_type,
        int _count,
        int _cancel_index
    ) {
        InternalItem item(
            _action,
            _unit_type,
            _tech_type,
            _upgrade_type,
            _count,
            _cancel_index
        );
        _insert_next(item);
    }
    
    int current_index() {
        return cur_index;
    }

    const Item &peek_next() {
        return items[cur_index];
    }

    const Item &next() {
        ++cur_index;
        printf("build order next item: %d\n", cur_index);
        return items[cur_index - 1];
    }

    const Item &get(int i) {
        return items[i];
    }

    unsigned size() {
        return items.size();
    }

    bool finished() {
        return (unsigned int)cur_index >= items.size();
    }

    void print(unsigned int start=0) {
        for (; start < items.size(); ++start) {
            print_item(start);
        }
    }

    void print_item(unsigned int i) {
        const Item &item = items[i];
        std::cout << "[" << i << "]: \n\t";
        switch(item.action) {
            case Item::MAKE:
                std::cout << "Make    " << item.count << " " << item.unit_type.c_str();
                break;
            case Item::MORPH:
                std::cout << "Morph   " << item.count << " " << item.unit_type.c_str();
                break;
            case Item::BUILD:
                std::cout << "Build   " << item.count << " " << item.unit_type.c_str();
                break;
            case Item::TECH:
                std::cout << "Tech    " << item.count << " " << item.tech_type.c_str();
                break;
            case Item::UPGRADE:
                std::cout << "Upgrade " << item.count << " " << item.upgrade_type.c_str();
                break;
            case Item::CANCEL:
                std::cout << "Cancel  " << item.count << " ";
                if (item.unit_type != BWAPI::UnitTypes::None) {
                    std::cout << item.unit_type;
                }
                else if (item.tech_type != BWAPI::TechTypes::None) {
                    std::cout << item.tech_type;
                }
                else {
                    std::cout << item.upgrade_type;
                }
        }
        std::cout << "\n\tCancel Index: " << item.cancel_index;
        std::cout << "\n\tMineral Cost: " << item.mineral_cost();
        std::cout << "\n\tGas Cost:     " << item.gas_cost();
        std::cout << "\n\tLarva Cost:   " << item.larva_cost();
        std::cout << "\n\tSupply Cost:  " << item.supply_cost() << std::endl;
    }
}