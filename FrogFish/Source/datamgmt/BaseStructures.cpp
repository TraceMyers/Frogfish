#include "BaseStructures.h"
#include <BWEM/bwem.h>
#include "../data/BaseStorage.h"
#include "../data/UnitStorage.h"

// TODO: add resource depots

// needs to be called after BaseOwnership::assign_new_bases()
void assign_structures_to_bases(
    BWEM::Map &the_map, 
    BaseStorage &base_storage,
    UnitStorage &unit_storage
) {
    const FBArray &self_bases = base_storage.get_self_bases();
    const FUArray &new_self_units = unit_storage.get_self_newly_stored();
    const FUArray &changed_type_self_units = unit_storage.get_self_newly_changed_type();
    self_assign_structures(the_map, base_storage, new_self_units, self_bases);
    self_assign_structures(the_map, base_storage, changed_type_self_units, self_bases);
    const EBArray &enemy_bases = base_storage.get_enemy_bases();
    const EUArray &new_enemy_units = unit_storage.get_enemy_newly_stored();
    const EUArray &changed_type_enemy_units = unit_storage.get_enemy_newly_changed_type();
    enemy_assign_structures(the_map, base_storage, new_enemy_units, enemy_bases);
    enemy_assign_structures(the_map, base_storage, changed_type_enemy_units, enemy_bases);
    if (Broodwar->enemy()->getRace() == BWAPI::Races::Terran) {
        const EUArray &changed_pos_enemy_units = unit_storage.get_enemy_newly_changed_pos();
        enemy_assign_structures(the_map, base_storage, changed_pos_enemy_units, enemy_bases);
    }
}

void self_assign_structures(
    BWEM::Map &the_map, 
    BaseStorage &base_storage,
    const FUArray &self_units,
    const FBArray &self_bases
) {
    for (register int i = 0; i < self_units.length(); i++) {
        FUnit f_unit = self_units[i];
        if (f_unit->is_struct()) {
            const TilePosition &structure_tilepos = f_unit->get_tilepos();
            if (the_map.Valid(structure_tilepos)) {
                const BWEM::Area *structure_area = the_map.GetArea(structure_tilepos);
                if (structure_area != nullptr) {
                    std::vector<FBase> potential_assign_bases;
                    for (int i = 0; i < self_bases.length(); i++) {
                        FBase f_base = self_bases[i];
                        if (structure_area->Id() == f_base->get_area()->Id()) {
                            potential_assign_bases.push_back(f_base);
                        }
                    }
                    int potential_assign_base_ct = potential_assign_bases.size();
                    if (potential_assign_base_ct == 1) {
                        FBase f_base = potential_assign_bases[0];
                        f_base->add_structure(f_unit);
                    }
                    else if (potential_assign_base_ct > 1) {
                        std::vector<double> distances;
                        const Position &struct_pos = f_unit->get_pos();
                        for (auto &base : potential_assign_bases) {
                            const Position &base_pos = base->get_center();
                            distances.push_back(
                                struct_pos.getApproxDistance(base_pos)
                            );
                        }
                        auto min_dist = std::min_element(distances.begin(), distances.end());
                        int base_i = std::distance(distances.begin(), min_dist);
                        const FBase f_base = potential_assign_bases[base_i];
                        f_base->add_structure(f_unit);
                    }
                }
            }
        }
    }
}

void enemy_assign_structures(
    BWEM::Map &the_map, 
    BaseStorage &base_storage,
    const EUArray &enemy_units,
    const EBArray &enemy_bases
) {
    for (register int i = 0; i < enemy_units.length(); i++) {
        EUnit e_unit = enemy_units[i];
        if (e_unit->is_struct()) {
            const TilePosition &structure_tilepos = e_unit->get_tilepos();
            if (the_map.Valid(structure_tilepos)) {
                const BWEM::Area *structure_area = the_map.GetArea(structure_tilepos);
                if (structure_area != nullptr) {
                    std::vector<EBase> potential_assign_bases;
                    for (int i = 0; i < enemy_bases.length(); i++) {
                        EBase e_base = enemy_bases[i];
                        if (structure_area->Id() == e_base->get_area()->Id()) {
                            potential_assign_bases.push_back(e_base);
                        }
                    }
                    int potential_assign_base_ct = potential_assign_bases.size();
                    if (potential_assign_base_ct == 1) {
                        EBase e_base = potential_assign_bases[0];
                        e_base->add_structure(e_unit);
                    }
                    else if (potential_assign_base_ct > 1) {
                        std::vector<double> distances;
                        const Position &struct_pos = e_unit->get_pos();
                        for (auto &base : potential_assign_bases) {
                            const Position &base_pos = base->get_center();
                            distances.push_back(
                                struct_pos.getApproxDistance(base_pos)
                            );
                        }
                        auto min_dist = std::min_element(distances.begin(), distances.end());
                        int base_i = std::distance(distances.begin(), min_dist);
                        const EBase e_base = potential_assign_bases[base_i];
                        e_base->add_structure(e_unit);
                    }
                }
            }
        }
    }
}

void unassign_base_structures(
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
        if (!e_unit->is_struct()) {
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