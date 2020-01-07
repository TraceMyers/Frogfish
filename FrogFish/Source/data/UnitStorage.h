#pragma once

#include "EnemyUnit.h"
#include "FrogUnit.h"
#include <map>
#include <iostream>
#include <vector>

class UnitStorage {

private:

    std::vector<BWAPI::Unit> store_buff;
    std::vector<BWAPI::Unit> remove_buff;

    std::map<int, FUnit> self_ID_2_funit;
    std::map<int, EUnit> enemy_ID_2_eunit;

    std::vector<FUnit> self_newly_stored;
    std::vector<FUnit> self_newly_removed;
    std::vector<FUnit> self_newly_changed_type;
    std::vector<EUnit> enemy_newly_stored;
    std::vector<EUnit> enemy_newly_removed;
    std::vector<EUnit> enemy_newly_changed_type;
    std::vector<EUnit> enemy_newly_changed_pos;

    void self_store(const BWAPI::Unit u) {
        FUnit f_unit;
        int ID = u->getID();

        if ((f_unit = self_ID_2_funit[ID]) == NULL) {
            f_unit = new FrogUnit(u);
            self_ID_2_funit[ID] = f_unit;
            self_newly_stored.push_back(f_unit);
        }
    }

    void enemy_store(const BWAPI::Unit u) {
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
            enemy_newly_stored.push_back(e_unit);
        }
    }

    void self_remove(const BWAPI::Unit u) {
        FUnit f_unit;
        int ID = u->getID();

        if ((f_unit = self_ID_2_funit[ID]) != NULL) {
            //printf("removing self unit %s\n", f_unit->get_name().c_str());
            self_ID_2_funit.erase(ID);
            self_newly_removed.push_back(f_unit);
        }
        else {
            printf("UnitStorage.self_remove() : tried to remove unit that isn't in storage\n");
        }
    }

    void enemy_remove(const BWAPI::Unit u) {
        EUnit e_unit;
        int ID = u->getID();

        if ((e_unit = enemy_ID_2_eunit[ID]) != NULL) {
            printf(
                "removing enemy unit %s, id %d\n", 
                e_unit->get_name().c_str(),
                e_unit->get_ID()
            );
            enemy_ID_2_eunit.erase(ID);
            enemy_newly_removed.push_back(e_unit);
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
        std::vector<Unit>::iterator it = 
            std::find(store_buff.begin(), store_buff.end(), u);
        bool unit_in_buff = (it != store_buff.end());
        if (!unit_in_buff) {
            store_buff.push_back(u);
        }
    }

    void queue_remove(const Unit u) {
        std::vector<Unit>::iterator it = 
            std::find(remove_buff.begin(), remove_buff.end(), u);
        bool unit_in_buff = (it != remove_buff.end());
        if (!unit_in_buff) {
            remove_buff.push_back(u);
        }
    }

    void store_queued() {
        register BWAPI::Unit u;
        for (unsigned int i = 0; i < store_buff.size(); i++) {
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
        register BWAPI::Unit u;
        for (register int i = 0; i < remove_buff.size(); i++) {
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
        for (register int i = 0; i < self_newly_removed.size(); i++) {
            delete self_newly_removed[i];
        }
        self_newly_removed.clear();
        self_newly_changed_type.clear();

        enemy_newly_stored.clear();
        for (register int i = 0; i < enemy_newly_removed.size(); i++) {
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
                self_newly_changed_type.push_back(f_unit);
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
                        enemy_newly_changed_type.push_back(e_unit);
                        if (e_unit->e_type != e_unit->STRUCT && u->getType().isBuilding()) {
                            e_unit->set_just_became_struct(true);
                        }
                    }
                }
                else if (e_unit->get_pos() != u->getPosition()) {
                    enemy_newly_changed_pos.push_back(e_unit);
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
                    enemy_newly_changed_pos.push_back(e_unit);
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

    const std::vector<FUnit> &get_self_newly_stored() {return self_newly_stored;}

    const std::vector<FUnit> &get_self_newly_removed() {return self_newly_removed;}

    const std::vector<FUnit> &get_self_newly_changed_type() {return self_newly_changed_type;}

    const std::vector<EUnit> &get_enemy_newly_stored() {return enemy_newly_stored;}

    const std::vector<EUnit> &get_enemy_newly_removed() {return enemy_newly_removed;}

    const std::vector<EUnit> &get_enemy_newly_changed_type() {return enemy_newly_changed_type;}

    const std::vector<EUnit> &get_enemy_newly_changed_pos() {return enemy_newly_changed_pos;}

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
    }
};