#pragma once

#include "EnemyUnit.h"
#include "FrogUnit.h"
#include "../utility/storage/EUArray.h"
#include "../utility/storage/FUArray.h"
#include "../utility/storage/IDArray.h"
#include "../utility/storage/UnitBuff.h"
#include "../utility/storage/FArray.h"
#include <map>
#include <BWAPI.h>
#include <iostream>

using namespace BWAPI;

class UnitStorage {

private:

    UnitBuff store_buff;
    UnitBuff remove_buff;

    std::map<int, FUnit> self_ID_2_funit;
    std::map<int, EUnit> enemy_ID_2_eunit;

    FUArray self_newly_stored;
    FUArray self_newly_removed;
    FUArray self_newly_changed_type;
    EUArray enemy_newly_stored;
    EUArray enemy_newly_removed;
    EUArray enemy_newly_changed_type;
    EUArray enemy_newly_changed_pos;

    void self_store(const Unit u) {
        FUnit f_unit;
        int ID = u->getID();

        if ((f_unit = self_ID_2_funit[ID]) == NULL) {
            f_unit = new FrogUnit(u);
            self_ID_2_funit[ID] = f_unit;
            self_newly_stored.add(f_unit);
        }
    }

    void enemy_store(const Unit u) {
        EUnit e_unit;
        int ID = u->getID();

        if ((e_unit = enemy_ID_2_eunit[ID]) == NULL) {
            printf(
                "storing enemy unit %s, id %d\n", 
                u->getType().getName().c_str(),
                u->getID()
            );
            e_unit = new EnemyUnit(u);
            enemy_ID_2_eunit[ID] = e_unit;
            enemy_newly_stored.add(e_unit);
        }
    }

    void self_remove(const Unit u) {
        FUnit f_unit;
        int ID = u->getID();

        if ((f_unit = self_ID_2_funit[ID]) != NULL) {
            //printf("removing self unit %s\n", f_unit->get_name().c_str());
            self_ID_2_funit.erase(ID);
            self_newly_removed.add(f_unit);
        }
        else {
            printf("UnitStorage.self_remove() : tried to remove unit that isn't in storage\n");
        }
    }

    void enemy_remove(const Unit u) {
        EUnit e_unit;
        int ID = u->getID();

        if ((e_unit = enemy_ID_2_eunit[ID]) != NULL) {
            printf(
                "removing enemy unit %s, id %d\n", 
                e_unit->get_name().c_str(),
                e_unit->get_ID()
            );
            enemy_ID_2_eunit.erase(ID);
            enemy_newly_removed.add(e_unit);
        }
        else {
            printf("UnitStorage.enemy_remove() : tried to remove unit that isn't in storage\n");
        }
    }

public:

    void update() {
        store_queued();
        remove_queued();
        update_self_units();
        update_enemy_units();
    }

    void queue_store(const Unit u) {
        if (!store_buff.has(u)) {
            store_buff.add(u);
        }
    }

    void queue_remove(const Unit u) {
        if (!remove_buff.has(u)) {
            remove_buff.add(u);
        }
    }

    void store_queued() {
        register Unit u;
        for (register int i = 0; i < store_buff.length(); i++) {
            u = store_buff[i];
            if (u->getPlayer() == Broodwar->self()) {
                self_store(u);
            }
            else if (u->getPlayer() == Broodwar->enemy()) {
                enemy_store(u);
            }
        }
        store_buff.clear();
    }

    void remove_queued() {
        register Unit u;
        for (register int i = 0; i < remove_buff.length(); i++) {
            u = remove_buff[i];
            if (u->getPlayer() == Broodwar->self()) {
                self_remove(u);
            }
            else if (u->getPlayer() == Broodwar->enemy()) {
                enemy_remove(u);
            }
        }
        remove_buff.clear();
    }

    void clear_newly_assigned() {
        self_newly_stored.clear();
        for (register int i = 0; i < self_newly_removed.length(); i++) {
            delete self_newly_removed[i];
        }
        self_newly_removed.clear();
        self_newly_changed_type.clear();

        enemy_newly_stored.clear();
        for (register int i = 0; i < enemy_newly_removed.length(); i++) {
            delete enemy_newly_removed[i];
        }
        enemy_newly_removed.clear();
        enemy_newly_changed_type.clear();
        enemy_newly_changed_pos.clear();
    }

    void update_self_units() {
        register std::map<int, FUnit>::iterator fu_it;
        register FUnit f_unit;
        for (fu_it = self_ID_2_funit.begin(); fu_it != self_ID_2_funit.end(); ++fu_it) {
            f_unit = fu_it->second;
            if (f_unit->get_type() != f_unit->bwapi_u()->getType()) {
                f_unit->update();
                self_newly_changed_type.add(f_unit);
            }
        }
    }

    void update_enemy_units() {
        register std::map<int, EUnit>::iterator eu_it;
        register EUnit e_unit;
        register Unit u;
        for (eu_it = enemy_ID_2_eunit.begin(); eu_it != enemy_ID_2_eunit.end(); ++eu_it) {
            e_unit = eu_it->second;
            u = e_unit->bwapi_u();
            if (u->isVisible()) {
                if (e_unit->get_type() != u->getType()) {
                    if (u->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser) {
                        enemy_remove(u);
                    }
                    else {
                        e_unit->update();
                        enemy_newly_changed_type.add(e_unit);
                        if (e_unit->e_type != e_unit->STRUCT && u->getType().isBuilding()) {
                            e_unit->set_just_became_struct(true);
                        }
                    }
                }
                else if (e_unit->get_pos() != u->getPosition()) {
                    enemy_newly_changed_pos.add(e_unit);
                    e_unit->update();
                }
                else {
                    e_unit->update();
                }
            }
            else if (
                !(e_unit->is_missing()) 
                && !(e_unit->bwapi_u()->isVisible())
                && (Broodwar->isVisible(e_unit->get_tilepos()))
            ) {
                if (e_unit->is_struct()) {
                    enemy_remove(e_unit->bwapi_u());
                }
                else {
                    enemy_newly_changed_pos.add(e_unit);
                }
                e_unit->set_missing(true);
            }
        }
    }

    const std::map<int, FUnit> &self_units() {
        return self_ID_2_funit;
    }

    const std::map<int, EUnit> &enemy_units() {
        return enemy_ID_2_eunit;
    }

    const FUArray &get_self_newly_stored() {return self_newly_stored;}

    const FUArray &get_self_newly_removed() {return self_newly_removed;}

    const FUArray &get_self_newly_changed_type() {return self_newly_changed_type;}

    const EUArray &get_enemy_newly_stored() {return enemy_newly_stored;}

    const EUArray &get_enemy_newly_removed() {return enemy_newly_removed;}

    const EUArray &get_enemy_newly_changed_type() {return enemy_newly_changed_type;}

    const EUArray &get_enemy_newly_changed_pos() {return enemy_newly_changed_pos;}

    void free_data() {
        // called only by FrogFish::onEnd()
        register std::map<int, FUnit>::iterator fu_it;
        register FUnit f_unit;
        for (fu_it = self_ID_2_funit.begin(); fu_it != self_ID_2_funit.end(); ++fu_it) {
            f_unit = fu_it->second;
            delete f_unit;
        }
        register std::map<int, EUnit>::iterator eu_it;
        register EUnit e_unit;
        for (eu_it = enemy_ID_2_eunit.begin(); eu_it != enemy_ID_2_eunit.end(); ++eu_it) {
            e_unit = eu_it->second;
            delete e_unit;
        }
        store_buff.free_data();
        remove_buff.free_data();
        self_newly_stored.free_data();
        self_newly_removed.free_data();
        self_newly_changed_type.free_data();
        enemy_newly_stored.free_data();
        enemy_newly_removed.free_data();
        enemy_newly_changed_type.free_data();
        enemy_newly_changed_pos.free_data();
    }
};