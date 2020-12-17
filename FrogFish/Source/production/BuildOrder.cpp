#include "BuildOrder.h"
#include "../basic/References.h"
#include "../test/TestMessage.h"
#include <BWAPI.h>
#include <fstream>
#include <thread>
#include <cassert>

using namespace Basic;

// TODO: change Build Order to linked list implementation to work better with manipulation
//      ^ outweighs befefits of random access?

namespace Production::BuildOrder { 

    Item::Item(
        ACTION act,
        BWAPI::UnitType u_type,
        BWAPI::TechType tch_type,
        BWAPI::UpgradeType up_type,
        int cancel_i

    ) :
        _action(act),
        _unit_type(u_type),
        _tech_type(tch_type),
        _upgrade_type(up_type),
        _cancel_index(cancel_i)
    {}

    namespace {

        struct InternalItem : public Item {

            InternalItem(
                ACTION act,
                BWAPI::UnitType u_type,
                BWAPI::TechType tch_type,
                BWAPI::UpgradeType up_type,
                int cancel_i
            ) : 
                Item (act, u_type, tch_type, up_type, cancel_i)
            {
                set_mineral_cost();
                set_gas_cost();
                set_supply_cost();
                set_larva_cost();
            }

            void set_mineral_cost() {
                switch(_action) {
                    case Item::ACTION::TECH:
                        _mineral_cost = _tech_type.mineralPrice();
                        break;
                    case Item::ACTION::UPGRADE:
                        _mineral_cost = _upgrade_type.mineralPrice();
                        break;
                    case Item::ACTION::CANCEL:
                        if (_unit_type != BWAPI::UnitTypes::None) {
                            if (_unit_type.isBuilding()) {
                                _mineral_cost = (int)(-_unit_type.mineralPrice() * 0.75);
                            }
                            else {
                                _mineral_cost = -_unit_type.mineralPrice();
                            }
                        }
                        else if (_tech_type != BWAPI::TechTypes::None) {
                            _mineral_cost = -_tech_type.mineralPrice();
                        }
                        else {
                            _mineral_cost = -_upgrade_type.mineralPrice();
                        }
                        break;
                    case Item::ACTION::OVERLORD_MAKE_BLOCK_OFF:
                    case Item::ACTION::OVERLORD_MAKE_BLOCK_ON:
                        _mineral_cost = 0;
                        break;
                    default:
                        _mineral_cost = _unit_type.mineralPrice();
                }
            }

            void set_gas_cost() {
                switch(_action) {
                    case Item::ACTION::TECH:
                        _gas_cost = _tech_type.gasPrice();
                        break;
                    case Item::ACTION::UPGRADE:
                        _gas_cost = _upgrade_type.gasPrice();
                        break;
                    case Item::ACTION::CANCEL:
                        if (_unit_type != BWAPI::UnitTypes::None) {
                            if (_unit_type.isBuilding()) {
                                _gas_cost = (int)(-_unit_type.gasPrice() * 0.75);
                            }
                            else {
                                _gas_cost = -_unit_type.gasPrice();
                            }
                        }
                        else if (_tech_type != BWAPI::TechTypes::None) {
                            _gas_cost = -_tech_type.gasPrice();
                        }
                        else {
                            _gas_cost = -_upgrade_type.gasPrice();
                        }
                        break;
                    case Item::ACTION::OVERLORD_MAKE_BLOCK_OFF:
                    case Item::ACTION::OVERLORD_MAKE_BLOCK_ON:
                        _gas_cost = 0;
                        break;
                    default:
                        _gas_cost = _unit_type.gasPrice();
                }
            }

            void set_supply_cost() {
                switch(_action) {
                    case Item::ACTION::BUILD:
                        _supply_cost = -2 - _unit_type.supplyProvided();
                        break;
                    case Item::ACTION::MORPH:
                        _supply_cost = 
                            _unit_type.supplyRequired() - _unit_type.whatBuilds().first.supplyRequired();
                        break;
                    case Item::ACTION::MAKE:
                        _supply_cost = _unit_type.supplyRequired() - _unit_type.supplyProvided();
                        break;
                    case Item::ACTION::CANCEL:
                        _supply_cost = _unit_type.supplyProvided() - _unit_type.supplyRequired()
                            + _unit_type.whatBuilds().first.supplyRequired();
                    default:
                        _supply_cost = 0;
                }
            }

            void set_larva_cost() {
                switch(_action) {
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
        std::string                 race;
        std::string                 name;
        std::vector<InternalItem>   items;
        int                         cur_index;
        Item                        end_item(
                                        Item::ACTION::NONE, 
                                        BWAPI::UnitTypes::None,
                                        BWAPI::TechTypes::None,
                                        BWAPI::UpgradeTypes::None,
                                        -1
                                    );
        bool                        overlord_make_block_on;

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
        overlord_make_block_on = false;
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
                            Item::ACTION _action;
                            BWAPI::UnitType _unit_type = BWAPI::UnitTypes::None;
                            BWAPI::TechType _tech_type = BWAPI::TechTypes::None;
                            BWAPI::UpgradeType _upgrade_type = BWAPI::UpgradeTypes::None;
                            int _count = 1;
                            int _cancel_index = -1;

                            in_file >> word;
                            word.pop_back();
                            if (word == "build") {
                                _action = Item::BUILD;
                            }
                            else if (word == "make") {
                                _action = Item::MAKE;
                            }
                            else if (word == "makeblockon") {
                                _action = Item::OVERLORD_MAKE_BLOCK_ON;
                            }
                            else if (word == "makeblockoff") {
                                _action = Item::OVERLORD_MAKE_BLOCK_OFF;
                            }
                            else if (word == "morph") {
                                _action = Item::MORPH;
                            }
                            else if (word == "tech") {
                                _action = Item::TECH;
                            }
                            else if (word == "upgrade") {
                                _action = Item::UPGRADE;
                            }
                            else if (word == "cancel") {
                                _action = Item::CANCEL;
                            }
                            else {
                                printf("BuildOrder error: _action = %s\n", word.c_str());
                            }

                            in_file >> word;
                            word.pop_back();
                            _count = std::stoi(word);

                            in_file >> word;
                            word.pop_back();
                            if (
                                _action == Item::BUILD
                                || _action == Item::MAKE
                                || _action == Item::MORPH
                                || _action == Item::CANCEL
                            ) {
                                for (int i = 0; i < Refs::Zerg::TYPE_CT; ++i) {
                                    if (word == Refs::Zerg::NAMES[i]) {
                                        _unit_type = Refs::Zerg::TYPES[i];
                                    }
                                }
                            }
                            else if (_action == Item::TECH || _action == Item::CANCEL) {
                                for (int i = 0; i < Refs::Zerg::TECH_CT; ++i) {
                                    if (word == Refs::Zerg::TECH_NAMES[i]) {
                                        _tech_type = Refs::Zerg::TECH_TYPES[i];
                                    }
                                }
                            }
                            else if (_action == Item::UPGRADE || _action == Item::CANCEL) {
                                for (int i = 0; i < Refs::Zerg::UPGRADE_CT; ++i) {
                                    if (word == Refs::Zerg::UPGRADE_NAMES[i]) {
                                        _upgrade_type = Refs::Zerg::UPGRADE_TYPES[i];
                                    }
                                }
                            }

                            in_file >> word;
                            if (word != "null") {
                                _cancel_index = std::stoi(word);
                            }

                            for (int i = 0; i < _count; ++i) {
                                push(
                                    _action,
                                    _unit_type,
                                    _tech_type,
                                    _upgrade_type,
                                    _cancel_index
                                );
                            }
                            
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
        Item::ACTION act,
        BWAPI::UnitType u_type,
        BWAPI::TechType tch_type,
        BWAPI::UpgradeType up_type,
        int cancel_i
    ) {
        InternalItem item(
            act,
            u_type,
            tch_type,
            up_type,
            cancel_i
        );
        _push(item);
    }

    void insert(
        Item::ACTION act,
        BWAPI::UnitType u_type,
        BWAPI::TechType tch_type,
        BWAPI::UpgradeType up_type,
        int cancel_i,
        int insert_index
    ) {
        InternalItem item(
            act,
            u_type,
            tch_type,
            up_type,
            cancel_i
        );
        _insert(item, insert_index);
    }

    void insert_next(
        Item::ACTION act,
        BWAPI::UnitType u_type,
        BWAPI::TechType tch_type,
        BWAPI::UpgradeType up_type,
        int cancel_i
    ) {
        InternalItem item(
            act,
            u_type,
            tch_type,
            up_type,
            cancel_i
        );
        _insert_next(item);
    }
    
    int current_index() {
        return cur_index;
    }

    const Item &current_item() {
        if (cur_index < items.size()) {return items[cur_index];}
        return end_item;
    }

    void next() {
        ++cur_index;
        if (cur_index < items.size()) {
            auto& cur_item = items[cur_index];
            const Item::ACTION& action = cur_item.action();
            if (action == Item::OVERLORD_MAKE_BLOCK_ON) {
                overlord_make_block_on = true;
                DBGMSG("BuildOrder::next(): Overlord make block *ON*\n");
                next();
            }
            else if (action == Item::OVERLORD_MAKE_BLOCK_OFF) {
                overlord_make_block_on = false;
                DBGMSG("BuildOrder::next(): Overlord make block *OFF*\n");
                next();
            }
            DBGMSG("BuildOrder::next(): advancing to %d\n", cur_index);
            return;
        }
        DBGMSG("BuildOrder::next(): Build order reached the end.\n");
    }

    // unsafe - can cause read access error
    const Item &get(int i) {
        return items[i];
    }

    void move(int from, int to) {
        auto& item = items[from];
        items.erase(items.begin() + from);
        items.insert(items.begin() + to, item);
    }

    bool overlord_make_block() {
        return overlord_make_block_on;
    }

    unsigned size() {
        return items.size();
    }

    bool finished() {
        return (unsigned)cur_index >= items.size();
    }

    void print(unsigned int start) {
        for (; start < items.size(); ++start) {
            print_item(start);
        }
    }
    
    void print_item(unsigned int i) {
        const Item &item = items[i];
        std::cout << "[" << i << "]: \n\t";
        switch(item.action()) {
            case Item::MAKE:
                std::cout << "Make    " << item.unit_type().c_str();
                break;
            case Item::OVERLORD_MAKE_BLOCK_OFF:
                std::cout << "Overlord make block *OFF* ";
                break;
            case Item::OVERLORD_MAKE_BLOCK_ON:
                std::cout << "Overlord make block *ON*  ";
                break;
            case Item::MORPH:
                std::cout << "Morph   " << item.unit_type().c_str();
                break;
            case Item::BUILD:
                std::cout << "Build   " << item.unit_type().c_str();
                break;
            case Item::TECH:
                std::cout << "Tech    " << item.tech_type().c_str();
                break;
            case Item::UPGRADE:
                std::cout << "Upgrade " << item.upgrade_type().c_str();
                break;
            case Item::CANCEL:
                std::cout << "Cancel  ";
                if (item.unit_type() != BWAPI::UnitTypes::None) {
                    std::cout << item.unit_type();
                }
                else if (item.tech_type() != BWAPI::TechTypes::None) {
                    std::cout << item.tech_type();
                }
                else {
                    std::cout << item.upgrade_type();
                }
        }
        std::cout << "\n\tCancel Index: " << item.cancel_index();
        std::cout << "\n\tMineral Cost: " << item.mineral_cost();
        std::cout << "\n\tGas Cost:     " << item.gas_cost();
        std::cout << "\n\tLarva Cost:   " << item.larva_cost();
        std::cout << "\n\tSupply Cost:  " << item.supply_cost() << std::endl;
    }
}