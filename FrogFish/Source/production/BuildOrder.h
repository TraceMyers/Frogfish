#pragma once
#pragma message("including BuildOrder")

#include <BWAPI.h>
#include <fstream>
#include <thread>

namespace Production::BuildOrder {
    
    struct Item {

        /* PUBLIC */
        public:

        enum ACTION {MAKE, MORPH, BUILD, TECH, UPGRADE, CANCEL};

        Item(
            ACTION act,
            BWAPI::UnitType u_type,
            BWAPI::TechType tch_type,
            BWAPI::UpgradeType up_type,
            int cnt,
            int cancel_i
        );

        int action() const {
            return _action;
        }

        BWAPI::UnitType unit_type() const {
            return _unit_type;
        }

        BWAPI::TechType tech_type() const {
            return _tech_type;
        }

        BWAPI::UpgradeType upgrade_type() const {
            return _upgrade_type;
        }

        int count() const {
            return _count;
        }

        int cancel_index() const {
            return _cancel_index;
        }

        int mineral_cost() const {
            return _mineral_cost;
        }

        int gas_cost() const {
            return _gas_cost;
        }

        int larva_cost() const {
            return _larva_cost;
        }

        int supply_cost() const {
            return _supply_cost;
        }

        /* PROTECTED */
        protected:

        ACTION _action;
        BWAPI::UnitType _unit_type;
        BWAPI::TechType _tech_type;
        BWAPI::UpgradeType _upgrade_type;
        int _count;
        int _cancel_index;
        int _mineral_cost = 0;
        int _gas_cost = 0;
        int _larva_cost = 0;
        int _supply_cost = 0;
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
    void            print(unsigned int start=0);
    void            print_item(unsigned int i);

}