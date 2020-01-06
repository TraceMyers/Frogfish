#include "BaseAssets.h"
#include <BWEM/bwem.h>
#include "../data/BaseStorage.h"
#include "../data/UnitStorage.h"

// TODO: add resource depots & workers
// TODO: full template refactor 
enum UNIT_STORE_TYPE {NEW, CHANGE_TYPE, CHANGE_POS, REMOVED};
UNIT_STORE_TYPE store_type;

void assign_base_assets(
    BWEM::Map &the_map, 
    BaseStorage &base_storage,
    UnitStorage &unit_storage
) {
    const FBArray &self_bases = base_storage.get_self_bases();

    store_type = NEW;
    const FUArray &new_self_units = unit_storage.get_self_newly_stored();
    assign_assets(the_map, base_storage, new_self_units, self_bases);
    store_type = CHANGE_TYPE;
    const FUArray &changed_type_self_units = unit_storage.get_self_newly_changed_type();
    assign_assets(the_map, base_storage, changed_type_self_units, self_bases);

    const EBArray &enemy_bases = base_storage.get_enemy_bases();

    store_type = NEW;
    const EUArray &new_enemy_units = unit_storage.get_enemy_newly_stored();
    assign_assets(the_map, base_storage, new_enemy_units, enemy_bases);
    store_type = CHANGE_TYPE;
    const EUArray &changed_type_enemy_units = unit_storage.get_enemy_newly_changed_type();
    assign_assets(the_map, base_storage, changed_type_enemy_units, enemy_bases);
    store_type = CHANGE_POS;
    const EUArray &changed_pos_enemy_units = unit_storage.get_enemy_newly_changed_pos();
    assign_assets(the_map, base_storage, changed_pos_enemy_units, enemy_bases);
}

template <class UnitArrayT, class BaseArrayT>
void assign_assets(
    BWEM::Map &the_map, 
    BaseStorage &base_storage,
    const UnitArrayT &units,
    const BaseArrayT &bases
) {
    if (bases.length() > 0) {
        for (register int i = 0; i < units.length(); i++) {
            auto unit = units[i];
            if (unit->is_struct()) {
                if (store_type == CHANGE_TYPE
                    && Broodwar->enemy() == unit->bwapi_u()->getPlayer()
                    && Broodwar->enemy()->getRace() == BWAPI::Races::Zerg
                    && unit->get_type().whatBuilds().first != BWAPI::UnitTypes::Zerg_Drone
                ) {
                    // do nothing
                }
                else {assign_structure(the_map, base_storage, units, bases, unit);}
            }
        }
    }
}

template <class UnitArrayT, class BaseArrayT, class UnitT>
void assign_structure(
    BWEM::Map &the_map, 
    BaseStorage &base_storage,
    const UnitArrayT &units,
    const BaseArrayT &bases,
    const UnitT &unit
) {
    const TilePosition &structure_tilepos = unit->get_tilepos();
    if (the_map.Valid(structure_tilepos)) {
        const BWEM::Area *structure_area = the_map.GetArea(structure_tilepos);
        if (structure_area != nullptr) {
            std::vector<decltype(bases[0])> potential_assign_bases;
            for (int i = 0; i < bases.length(); i++) {
                auto base = bases[i];
                if (structure_area->Id() == base->get_area()->Id()) {
                    potential_assign_bases.push_back(base);
                }
            }
            int potential_assign_base_ct = potential_assign_bases.size();
            if (potential_assign_base_ct == 1) {
                auto base = potential_assign_bases[0];
                base->add_structure(unit);
            }
            else if (potential_assign_base_ct > 1) {
                std::vector<double> distances;
                const Position &struct_pos = unit->get_pos();
                for (auto &base : potential_assign_bases) {
                    const Position &base_pos = base->get_center();
                    distances.push_back(
                        struct_pos.getApproxDistance(base_pos)
                    );
                }
                auto min_dist = std::min_element(distances.begin(), distances.end());
                int base_i = std::distance(distances.begin(), min_dist);
                const auto base = potential_assign_bases[base_i];
                base->add_structure(unit);
            }
        }
    }
}


void unassign_base_assets(
    BWEM::Map &the_map, 
    BaseStorage &base_storage, 
    UnitStorage &unit_storage
) {
    const FBArray &self_bases = base_storage.get_self_bases();
    const FUArray &removed_self_units = unit_storage.get_self_newly_removed();
    unassign_self_structures_conditionless(self_bases, removed_self_units);
    const FUArray &changed_type_self_units = unit_storage.get_self_newly_changed_type();
    unassign_self_structures(self_bases, changed_type_self_units);

    const BWAPI::Race &enemy_race = Broodwar->enemy()->getRace();
    const EBArray &enemy_bases = base_storage.get_enemy_bases();
    const EUArray &removed_enemy_units = unit_storage.get_enemy_newly_removed();
    unassign_enemy_structures_conditionless(enemy_bases, removed_enemy_units);
    if (enemy_race == BWAPI::Races::Zerg) {
        const EUArray &changed_type_enemy_units = unit_storage.get_enemy_newly_changed_type();
        unassign_enemy_structures_zerg(enemy_bases, changed_type_enemy_units);
        const EUArray &changed_pos_enemy_units = unit_storage.get_enemy_newly_changed_pos();
        unassign_enemy_structures_terran(the_map, enemy_bases);
    }
    else if (enemy_race == BWAPI::Races::Terran) {
        const EUArray &changed_pos_enemy_units = unit_storage.get_enemy_newly_changed_pos();
        unassign_enemy_structures_terran(the_map, enemy_bases);
    }
}

void unassign_self_structures(const FBArray &self_bases, const FUArray &self_units) {
    for (register int i = 0; i < self_units.length(); i++) {
        FUnit f_unit = self_units[i];
        if (!f_unit->is_struct()) {
            for (register int j = 0; j < self_bases.length(); j++) {
                FBase f_base = self_bases[j];
                f_base->remove_structure(f_unit);
            }
        }
    }
}

void unassign_self_structures_conditionless(
    const FBArray &self_bases, 
    const FUArray &self_units
) {
    for (register int i = 0; i < self_units.length(); i++) {
        FUnit f_unit = self_units[i];
        for (register int j = 0; j < self_bases.length(); j++) {
            FBase f_base = self_bases[j];
            f_base->remove_structure(f_unit);
        }
    }
}

void unassign_enemy_structures_zerg(const EBArray &enemy_bases, const EUArray &enemy_units) {
    for (register int i = 0; i < enemy_units.length(); i++) {
        EUnit e_unit = enemy_units[i];
        if (!(e_unit->is_struct())) {
            for (register int j = 0; j < enemy_bases.length(); j++) {
                EBase e_base = enemy_bases[j];
                e_base->remove_structure(e_unit);
            }
        }
    }
}

void unassign_enemy_structures_terran(BWEM::Map &the_map, const EBArray &enemy_bases) {
    for (register int i = 0; i < enemy_bases.length(); i++) {
        EBase e_base = enemy_bases[i];
        const EUArray &base_structs = e_base->get_structures();
        for (register int j = 0; j < base_structs.length(); j++) {
            const EUnit structure = base_structs[i];
            const BWEM::Area *struct_area = the_map.GetArea(structure->get_tilepos());
            if (struct_area->Id() != e_base->get_area()->Id()) {
                e_base->remove_structure(structure);
            }
        }
    }
}

void unassign_enemy_structures_conditionless(
    const EBArray &enemy_bases, 
    const EUArray &enemy_units
) {
    for (register int i = 0; i < enemy_units.length(); i++) {
        EUnit e_unit = enemy_units[i];
        for (register int j = 0; j < enemy_bases.length(); j++) {
            EBase e_base = enemy_bases[j];
            e_base->remove_structure(e_unit);
        }
    }
}

// self assign structures
        
// enemy assign structures
/*
        
            */