#ifndef UNIT_STORAGE
#define UNIT_STORAGE

#include "EnemyUnit.h"
#include <list>
#include <map>
#include <BWAPI.h>
#include <iostream>

using namespace BWAPI;
using namespace Filter;

// TODO: need an enemy list for type Unknown

class UnitStorage {

private:

    enum STORECODE {NEW_UNIT, CHANGED_TYPE, NO_CHANGE};

    std::list<BWAPI::Unit> store_buffer;
    std::list<int> store_buffer_IDs;
    std::list<BWAPI::Unit> remove_buffer;
    std::list<int> remove_buffer_IDs;

    std::list<BWAPI::Unit> self_eggs;
    std::list<BWAPI::Unit> self_larva;
    std::list<BWAPI::Unit> self_workers;
    std::list<BWAPI::Unit> self_army;
    std::list<BWAPI::Unit> self_structures;
    std::map<int, std::list<BWAPI::Unit> *> self_ID_to_list;
    std::list<EnemyUnit> enemy_workers;
    std::list<EnemyUnit> enemy_army;
    std::list<EnemyUnit> enemy_structures;
    std::list<EnemyUnit> *enemy_lists[3] {&enemy_workers, &enemy_army, &enemy_structures};
    std::map<int, std::list<EnemyUnit> *> enemy_ID_to_list;   

    std::list<BWAPI::Unit> self_newly_stored;
    std::list<BWAPI::Unit> self_newly_moved;
    std::list<BWAPI::Unit> self_newly_removed;
    std::list<EnemyUnit> enemy_newly_stored;
    std::list<EnemyUnit> enemy_newly_moved;
    std::list<EnemyUnit> enemy_newly_removed;

    void self_store(const BWAPI::Unit u) {
        const int u_id = u->getID();    
        STORECODE store_code = NO_CHANGE;
        bool id_already_stored = self_ID_to_list[u_id] != NULL;

        if (id_already_stored) {
            std::list<BWAPI::Unit> u_list = *(self_ID_to_list[u_id]);
            u_list.remove(u);
            self_newly_moved.push_back(u);
            store_code = CHANGED_TYPE;
        }
        else {
            store_code = NEW_UNIT;
        }

        BWAPI::UnitType u_type = u->getType();
        switch(store_code){
            case NEW_UNIT:
                self_newly_stored.push_back(u);
            case CHANGED_TYPE:
                if (u_type.isBuilding()) {
                    self_structures.push_back(u);
                    self_ID_to_list[u_id] = &self_structures;
                }
                else if (u_type.isWorker()) {
                    self_workers.push_back(u);
                    self_ID_to_list[u_id] = &self_workers;
                }
                else if (u_type.canAttack() || u_type.isSpellcaster() || u_type.isFlyer()) {
                    self_army.push_back(u);
                    self_ID_to_list[u_id] = &self_army;
                }
                else if (u->isMorphing()) {
                    self_eggs.push_back(u);
                    self_ID_to_list[u_id] = &self_eggs;
                }
                else {
                    self_larva.push_back(u);
                    self_ID_to_list[u_id] = &self_larva;
                }
        }
    }

    void enemy_store(const BWAPI::Unit u) {
        EnemyUnit *enemy_unit_ptr;
        BWAPI::UnitType u_type = u->getType();
        STORECODE store_code = NO_CHANGE;
        const int u_id = u->getID();
        bool id_already_stored = enemy_ID_to_list[u_id] != NULL;

		if (id_already_stored) {
			std::list<EnemyUnit>::iterator e_unit_it;
			std::list<EnemyUnit>& enemy_unit_list = *(enemy_ID_to_list[u_id]);
			for (
				e_unit_it = enemy_unit_list.begin();
				e_unit_it != enemy_unit_list.end();
				++e_unit_it
				) {
				EnemyUnit& stored_unit = *e_unit_it;
				if (stored_unit.getID() == u_id) {
					if (u_type != stored_unit.getType()) {
						store_code = CHANGED_TYPE;
						enemy_unit_ptr = &stored_unit;
						enemy_unit_list.remove(stored_unit);
						enemy_newly_moved.push_back(stored_unit);
						//stored_unit.update();
					}
					break;
				}
			}
		}
		else {
			printf("new enemy unit: %s\n", u_type.getName().c_str());
			enemy_unit_ptr = new EnemyUnit(u);
			store_code = NEW_UNIT;
		}
		switch (store_code) {
		case NEW_UNIT:
			enemy_newly_stored.push_back(*enemy_unit_ptr);
		case CHANGED_TYPE:
			if (u_type.isBuilding() || u_type.isAddon()) {
				enemy_structures.push_back(*enemy_unit_ptr);
				enemy_ID_to_list[u_id] = &enemy_structures;
			}
			else if (u_type.isWorker()) {
				enemy_workers.push_back(*enemy_unit_ptr);
				enemy_ID_to_list[u_id] = &enemy_workers;
			}
			else {
				enemy_army.push_back(*enemy_unit_ptr);
				enemy_ID_to_list[u_id] = &enemy_army;
			}
		}
	
    }

    void self_remove(const BWAPI::Unit u) {
        register std::map<int, std::list<BWAPI::Unit> *>::iterator id_it;
        bool unit_in_storage = false;
        int u_id = u->getID();

        for (id_it = self_ID_to_list.begin(); id_it != self_ID_to_list.end(); ++id_it) {
            if (u_id == id_it->first) {
                unit_in_storage = true;
                break;
            }            
        }
        if (unit_in_storage) {
            (*(self_ID_to_list[u_id])).remove(u);
            self_ID_to_list.erase(u_id);
            self_newly_removed.push_back(u);
        }
    }

    void enemy_remove(const BWAPI::Unit u) {
        register std::map<int, std::list<EnemyUnit> *>::iterator id_it;
        bool unit_in_storage = false;
        int u_id = u->getID();

        for (id_it = enemy_ID_to_list.begin(); id_it != enemy_ID_to_list.end(); ++id_it) {
            if (u_id == id_it->first) {
                unit_in_storage = true;
                break;
            }            
        }
        if (unit_in_storage) {
            register std::list<EnemyUnit>::iterator e_unit_it;
            register EnemyUnit *stored_eu_ptr;
            std::list<EnemyUnit> enemy_unit_list = *(enemy_ID_to_list[u_id]);
            for (
                e_unit_it = enemy_unit_list.begin(); 
                e_unit_it != enemy_unit_list.end();
                ++e_unit_it
            ) {
                stored_eu_ptr = &(*e_unit_it);
                if ((*stored_eu_ptr).getID() == u_id) {
                    (*(enemy_ID_to_list[u_id])).remove(*stored_eu_ptr);
                    enemy_ID_to_list.erase(u_id);
                    enemy_newly_removed.push_back(*stored_eu_ptr);
                    break;
                }
            }
        }
    }

public:

    void queue_store(const BWAPI::Unit u) {
        register std::list<int>::iterator id_it;
        const int u_id = u->getID();
        bool already_buffered = false;

        for (id_it = store_buffer_IDs.begin(); id_it != store_buffer_IDs.end(); ++id_it) {
            if (*id_it == u_id) {
                already_buffered = true;
                break;
            }
        }
        if (!already_buffered) {
            store_buffer.push_back(u);
            store_buffer_IDs.push_back(u_id);
        }
    }

    void queue_remove(const BWAPI::Unit u) {
        register std::list<int>::iterator id_it;
        const int u_id = u->getID();
        bool already_buffered = false;

        for (id_it = remove_buffer_IDs.begin(); id_it != remove_buffer_IDs.end(); ++id_it) {
            if (*id_it == u_id) {
                already_buffered = true;
                break;
            }
        }
        if (!already_buffered) {
            remove_buffer.push_back(u);
            remove_buffer_IDs.push_back(u_id);
        }
    }

    void store_queued() {
        if (store_buffer.size() > 0) {
            register std::list<BWAPI::Unit>::iterator unit_it;

            for (unit_it = store_buffer.begin(); unit_it != store_buffer.end(); ++unit_it) {
                const BWAPI::Unit u = *unit_it;
				if (u->isDetected()) {
					const BWAPI::Player owner = u->getPlayer();
					if (owner == Broodwar->self()) {
						self_store(u);
					}
					else if (owner == Broodwar->enemy()) {
						enemy_store(u);
					}
                }
            }
            store_buffer.clear();
            store_buffer_IDs.clear();
        }
    }

    void remove_queued() {
        if (remove_buffer.size() > 0) {
            register std::list<BWAPI::Unit>::iterator unit_it;

            for (unit_it = remove_buffer.begin(); unit_it != remove_buffer.end(); ++unit_it) {
                const BWAPI::Unit u = *unit_it;
                const BWAPI::Player owner = u->getPlayer();
                if (owner == Broodwar->self()) {
                    self_remove(u);
                }
                else if (owner == Broodwar->enemy()) {
                    enemy_remove(u);
                }
            }
            remove_buffer.clear();
            remove_buffer_IDs.clear();
        }
    }

    void clear_newly_assigned() {
        self_newly_stored.clear();
        self_newly_removed.clear();
        self_newly_moved.clear();
        enemy_newly_stored.clear();
        register std::list<EnemyUnit>::iterator e_unit_it;
        for (
            e_unit_it = enemy_newly_removed.begin();
            e_unit_it != enemy_newly_removed.end();
            ++e_unit_it
        ) {
            delete &(*e_unit_it);
        }
        enemy_newly_removed.clear();
        enemy_newly_moved.clear();
    }

    void update_enemy_units() {
        register std::list<EnemyUnit>::iterator unit_it;
        for (int i = 0; i < 3; i++) {
            std::list<EnemyUnit> e_list = (*(enemy_lists[i]));
            printf("%d size: %lu\n", i, e_list.size());
            for (unit_it = e_list.begin(); unit_it != e_list.end(); ++unit_it) {
                EnemyUnit enemy_unit = *unit_it;
                BWAPI::Unit bw_unit = enemy_unit.getUnitPtr();
                BWAPI::UnitType u_type = bw_unit->getType();
                //printf("checking unit of type %s\n", u_type.getName().c_str());
                if (bw_unit->isVisible() && bw_unit->exists()) {
                    printf("visible unit: %s\n", enemy_unit.getType().getName().c_str());
                    //enemy_unit.update();
                }
            }
        }
    }

    std::list<BWAPI::Unit>& get_self_newly_stored() {return self_newly_stored;}

    std::list<BWAPI::Unit>& get_self_newly_removed() {return self_newly_removed;}

    std::list<EnemyUnit>& get_enemy_newly_stored() {return enemy_newly_stored;}

    std::list<EnemyUnit>& get_enemy_newly_removed() {return enemy_newly_removed;}

    std::list<BWAPI::Unit>& get_self_eggs() {return self_eggs;}

    std::list<BWAPI::Unit>& get_self_larva() {return self_larva;}

    std::list<BWAPI::Unit>& get_self_workers() {return self_workers;}

    std::list<BWAPI::Unit>& get_self_army() {return self_army;}

    std::list<BWAPI::Unit>& get_self_structures() {return self_structures;}

    std::list<EnemyUnit>& get_enemy_workers() {return enemy_workers;}

    std::list<EnemyUnit>& get_enemy_army() {return enemy_army;}

    std::list<EnemyUnit>& get_enemy_structures() {return enemy_structures;}

    bool self_unit_is_army(int ID) {return true;}
};

#endif