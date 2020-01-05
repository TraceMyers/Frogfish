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
void self_assign_new_bases(
    BWEM::Map &the_map, 
    BaseStorage &base_storage, 
    UnitStorage &unit_storage
) {
    const FUArray &new_self_units = unit_storage.get_self_newly_stored();
    const FUArray &changed_self_units = unit_storage.get_self_newly_changed_type();
    const BWEMBArray &neutral_bases = base_storage.get_neutral_bases();
    self_assign_new_bases_iter(the_map, base_storage, new_self_units, neutral_bases);
    self_assign_new_bases_iter(the_map, base_storage, changed_self_units, neutral_bases);
}

void self_assign_new_bases_iter(
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
                        std::vector<float> distances;
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