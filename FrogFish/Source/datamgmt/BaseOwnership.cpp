#include "../data/BaseStorage.h"
#include "../data/UnitStorage.h"
#include "BaseOwnership.h"
#include <BWAPI.h>
#include <BWEM/bwem.h>

using namespace BWAPI;

void init_base_storage(BWEM::Map &the_map, BaseStorage &base_storage) {
    const std::vector<BWEM::Area> &areas = the_map.Areas();
    int i = 0;
    for (auto &area : areas) {
        const std::vector<BWEM::Base> &bases = area.Bases();
        for (auto &base : bases) {
            base_storage.add_neutral_base(&base);
            i++;
        }
    }
    printf("BaseOwnership.init_base_storage(): Added %d neutral bases\n", i);
}

// *must* be called after UnitStorage::update()
// and before UnitStorage::clear_newly_assigned()
void assign_new_bases(
    BWEM::Map &the_map, 
    BaseStorage &base_storage, 
    UnitStorage &unit_storage
) {
    const BWEMBArray &neutral_bases = base_storage.get_neutral_bases();

    const FUArray &new_self_units = unit_storage.get_self_newly_stored();
    self_assign_new_bases(the_map, base_storage, new_self_units, neutral_bases);
    const FUArray &changed_type_self_units = unit_storage.get_self_newly_changed_type();
    self_assign_new_bases(the_map, base_storage, changed_type_self_units, neutral_bases);

    const BWAPI::Race &enemy_race = Broodwar->enemy()->getRace();
    const EUArray &new_enemy_units = unit_storage.get_enemy_newly_stored();
    enemy_assign_new_bases(the_map, base_storage, new_enemy_units, neutral_bases);
    if (enemy_race == BWAPI::Races::Zerg) {
        const EUArray &changed_type_enemy_units = unit_storage.get_enemy_newly_changed_type();
        enemy_assign_new_bases(the_map, base_storage, changed_type_enemy_units, neutral_bases);
    }
    else if (enemy_race == BWAPI::Races::Terran) {
        const EUArray &changed_pos_enemy_units = unit_storage.get_enemy_newly_changed_pos();
        enemy_assign_new_bases(the_map, base_storage, changed_pos_enemy_units, neutral_bases);
    }
}

void self_assign_new_bases(
    BWEM::Map &the_map, 
    BaseStorage &base_storage, 
    const FUArray &self_units,
    const BWEMBArray &neutral_bases
) {
    for (register int i = 0; i < self_units.length(); i++) {
        FUnit f_unit = self_units[i];
        if (f_unit->is_struct()) {
            const TilePosition &structure_tilepos = f_unit->get_tilepos();
            if (the_map.Valid(structure_tilepos)) {
                const BWEM::Area *structure_area = the_map.GetArea(structure_tilepos);
                if (structure_area != nullptr) {
                    const std::vector<BWEM::Base> &area_bases = structure_area->Bases();
                    std::vector<const BWEM::Base *> potential_new_bases;
                    for (auto &base : area_bases) {
                        int neutral_base_i = neutral_bases.find(&base);
                        if (neutral_base_i != -1) {
                            potential_new_bases.push_back(&base);
                        }
                    }
                    int potential_new_base_ct = potential_new_bases.size();
                    if (potential_new_base_ct == 1) {
                        const BWEM::Base *self_new_base = potential_new_bases[0];
                        base_storage.add_self_base(self_new_base);
                    }
                    else if (potential_new_base_ct > 1) {
                        std::vector<double> distances;
                        const Position &struct_pos = f_unit->get_pos();
                        for (auto &base : potential_new_bases) {
                            const Position &base_pos = base->Center();
                            distances.push_back(
                                struct_pos.getApproxDistance(base_pos)
                            );
                        }
                        auto min_dist = std::min_element(distances.begin(), distances.end());
                        int base_i = std::distance(distances.begin(), min_dist);
                        const BWEM::Base *self_new_base = potential_new_bases[base_i];
                        base_storage.add_self_base(self_new_base);
                    }
                }
            }
        }
    }
}

void enemy_assign_new_bases(
    BWEM::Map &the_map, 
    BaseStorage &base_storage, 
    const EUArray &enemy_units,
    const BWEMBArray &neutral_bases
) {
    for (register int i = 0; i < enemy_units.length(); i++) {
        EUnit e_unit = enemy_units[i];
        if (e_unit->is_struct()) {
            const TilePosition &structure_tilepos = e_unit->get_tilepos();
            if (the_map.Valid(structure_tilepos)) {
                const BWEM::Area *structure_area = the_map.GetArea(structure_tilepos);
                if (structure_area != nullptr) {
                    const std::vector<BWEM::Base> &area_bases = structure_area->Bases();
                    std::vector<const BWEM::Base *> potential_new_bases;
                    for (auto &base : area_bases) {
                        int neutral_base_i = neutral_bases.find(&base);
                        if (neutral_base_i != -1) {
                            potential_new_bases.push_back(&base);
                        }
                    }
                    int potential_new_base_ct = potential_new_bases.size();
                    if (potential_new_base_ct == 1) {
                        const BWEM::Base *enemy_new_base = potential_new_bases[0];
                        base_storage.add_enemy_base(enemy_new_base);
                    }
                    else if (potential_new_base_ct > 1) {
                        std::vector<double> distances;
                        const Position &struct_pos = e_unit->get_pos();
                        for (auto &base : potential_new_bases) {
                            const Position &base_pos = base->Center();
                            distances.push_back(
                                struct_pos.getApproxDistance(base_pos)
                            );
                        }
                        auto min_dist = std::min_element(distances.begin(), distances.end());
                        int base_i = std::distance(distances.begin(), min_dist);
                        const BWEM::Base *enemy_new_base = potential_new_bases[base_i];
                        base_storage.add_enemy_base(enemy_new_base);
                    }
                }
            }
        }
    }
}

// must either call before assign_new_bases()
// or call after both assign_new_bases() and
// the function that adds structures to bases
void unassign_bases(BaseStorage &base_storage) {
    const FBArray &self_bases = base_storage.get_self_bases();
    std::vector<FBase> remove_self_bases;
    for (int i = 0; i < self_bases.length(); i++) {
        const FBase f_base = self_bases[i];
        if (f_base->get_structure_ct() == 0) {
            remove_self_bases.push_back(f_base);
        }
    }
    for (auto &f_base : remove_self_bases) {
        base_storage.remove_self_base(f_base);
    }
    const EBArray &enemy_bases = base_storage.get_enemy_bases();
    std::vector<EBase> remove_enemy_bases;
    for (int i = 0; i < enemy_bases.length(); i++) {
        const EBase e_base = enemy_bases[i];
        if (e_base->get_structure_ct() == 0) {
            remove_enemy_bases.push_back(e_base);
        }
    }
    for (auto &e_base : remove_enemy_bases) {
        base_storage.remove_enemy_base(e_base);
    }
}