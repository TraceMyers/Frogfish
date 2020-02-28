#pragma once
#pragma message("including BuildOrder")

#include <BWAPI.h>
#include <fstream>
#include <thread>

namespace Production::BuildOrder {
    
    struct Item {

        /* PUBLIC */
        public:

        const enum ACTION {MAKE, MORPH, BUILD, TECH, UPGRADE, CANCEL} action;

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

        int mineral_cost() {
            return _mineral_cost;
        }

        int gas_cost() {
            return _gas_cost;
        }

        int larva_cost() {
            return _larva_cost;
        }

        int supply_cost() {
            return _supply_cost;
        }

        /* PROTECTED */
        protected:

        int _mineral_cost;
        int _gas_cost;
        int _larva_cost;
        int _supply_cost;
    };

    void            load(const char *_race, const char *build_name);
    void            push(
                        Item::ACTION _action,
                        BWAPI::UnitType _unit_type,
                        BWAPI::TechType _tech_type,
                        BWAPI::UpgradeType _upgrade_type,
                        int _count,
                        int _cancel_index
                    );
    void            insert(
                        Item::ACTION _action,
                        BWAPI::UnitType _unit_type,
                        BWAPI::TechType _tech_type,
                        BWAPI::UpgradeType _upgrade_type,
                        int _count,
                        int _cancel_index,
                        int insert_index
                    );
    void            insert_next(
                        Item::ACTION _action,
                        BWAPI::UnitType _unit_type,
                        BWAPI::TechType _tech_type,
                        BWAPI::UpgradeType _upgrade_type,
                        int _count,
                        int _cancel_index
                    );
    int             current_index();
    const Item &    peek_next();
    const Item &    next();
    const Item &    get(int i);
    unsigned        size();
    bool            finished();
    void            print();

}