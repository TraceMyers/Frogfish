// the problem is here
#include "BaseAssets.h"
#include <BWEM/bwem.h>
#include "../data/BaseStorage.h"
#include "../data/UnitStorage.h"

// TODO: add resource depots & workers
enum UNIT_STORE_TYPE {NEW, CHANGE_TYPE, CHANGE_POS, REMOVED};
UNIT_STORE_TYPE store_type;

void assign_base_assets(
    BWEM::Map &the_map, 
    BaseStorage &base_storage,
    UnitStorage &unit_storage
) {
    /*
    const std::vector<FBase> &self_bases = base_storage.get_self_bases();

    store_type = NEW;
    const std::vector<FUnit> &new_self_units = unit_storage.get_self_newly_stored();
    assign_assets(the_map, base_storage, new_self_units, self_bases);
    store_type = CHANGE_TYPE;
    const std::vector<FUnit> &changed_type_self_units = unit_storage.get_self_newly_changed_type();
    assign_assets(the_map, base_storage, changed_type_self_units, self_bases);

    const std::vector<EBase> &enemy_bases = base_storage.get_enemy_bases();

    store_type = NEW;
    const std::vector<EUnit> &new_enemy_units = unit_storage.get_enemy_newly_stored();
    assign_assets(the_map, base_storage, new_enemy_units, enemy_bases);
    store_type = CHANGE_TYPE;
    const std::vector<EUnit> &changed_type_enemy_units = unit_storage.get_enemy_newly_changed_type();
    assign_assets(the_map, base_storage, changed_type_enemy_units, enemy_bases);
    store_type = CHANGE_POS;
    const std::vector<EUnit> &changed_pos_enemy_units = unit_storage.get_enemy_newly_changed_pos();
    assign_assets(the_map, base_storage, changed_pos_enemy_units, enemy_bases);
    */
}

template <class UnitArrayT, class BaseArrayT>
void assign_assets(
    BWEM::Map &the_map, 
    BaseStorage &base_storage,
    const UnitArrayT &units,
    const BaseArrayT &bases
) {
    /*
    if (bases.size() > 0) {
        for (register int i = 0; i < units.size(); i++) {
            auto unit = units[i];
            if (unit->is_struct()) {
                if (store_type == CHANGE_TYPE
                    && Broodwar->enemy() == unit->bwapi_u()->getPlayer()
                    && Broodwar->enemy()->getRace() == BWAPI::Races::Zerg
                    && unit->get_type().whatBuilds().first != BWAPI::UnitTypes::Zerg_Drone
                ) {
                    ;
                }
                else {assign_structure(the_map, base_storage, units, bases, unit);}
            }
        }
    }
    */
}

template <class UnitArrayT, class BaseArrayT, class UnitT>
void assign_structure(
    BWEM::Map &the_map, 
    BaseStorage &base_storage,
    const UnitArrayT &units,
    const BaseArrayT &bases,
    const UnitT &unit
) {
    /*
    const TilePosition &structure_tilepos = unit->get_tilepos();
    if (the_map.Valid(structure_tilepos)) {
        const BWEM::Area *structure_area = the_map.GetArea(structure_tilepos);
        if (structure_area != nullptr) {
            std::vector<decltype(bases[0])> potential_assign_bases;
            for (int i = 0; i < bases.size(); i++) {
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
    */
}

void unassign_base_assets(
    BWEM::Map &the_map, 
    BaseStorage &base_storage, 
    UnitStorage &unit_storage
) {
    /*
    const std::vector<FBase> &self_bases = base_storage.get_self_bases();

    const std::vector<FUnit> &removed_self_units = unit_storage.get_self_newly_removed();
    unassign_assets_unconditional(self_bases, removed_self_units);
    const std::vector<FUnit> &changed_type_self_units = unit_storage.get_self_newly_changed_type();
    unassign_assets(self_bases, changed_type_self_units);

    const std::vector<EBase> &enemy_bases = base_storage.get_enemy_bases();

    const std::vector<EUnit> &removed_enemy_units = unit_storage.get_enemy_newly_removed();
    unassign_assets_unconditional(enemy_bases, removed_enemy_units);
    if (Broodwar->enemy()->getRace() == BWAPI::Races::Zerg) {
        const std::vector<EUnit> &changed_type_enemy_units = unit_storage.get_enemy_newly_changed_type();
        unassign_assets(enemy_bases, changed_type_enemy_units);
    }

    unassign_wrong_area_assets(the_map, enemy_bases);
    */
}

template <class BaseArrayT, class UnitArrayT>
void unassign_assets(const BaseArrayT &bases, const UnitArrayT &units) {
    /*
    for (register int i = 0; i < units.size(); i++) {
        auto unit = units[i];
        if (!unit->is_struct()) {
            for (register int j = 0; j < bases.size(); j++) {
                auto base = bases[j];
                base->remove_structure(unit);
            }
            // needs more
        }
        else if (unit->is_missing()) {
            for (register int j = 0; j < bases.size(); j++) {
                auto base = bases[j];
                base->remove_structure(unit);
            }
        }
    }
    */
}

template <class BaseArrayT, class UnitArrayT>
void unassign_assets_unconditional(
    const BaseArrayT &bases, 
    const UnitArrayT &units
) {
    /*
    for (register int i = 0; i < units.size(); i++) {
        auto unit = units[i];
        for (register int j = 0; j < bases.size(); j++) {
            auto base = bases[j];
            base->remove_structure(unit);
            base->remove_worker(unit);
            base->remove_resource_depot(unit);
        }
    }
    */
}

template <class BaseArrayT>
void unassign_wrong_area_assets(BWEM::Map &the_map, const BaseArrayT &bases) {
    /*
    for (register int i = 0; i < bases.size(); i++) {
        auto base = bases[i];
        auto &base_structs = base->get_structures();
        unassign_wrong_area_structure(the_map, base, base_structs);
        auto &base_workers = base->get_workers();
        unassign_wrong_area_worker(the_map, base, base_workers);
    }
    */
}

template <class BaseT, class UnitArrayT>
void unassign_wrong_area_structure(BWEM::Map &the_map, BaseT base, UnitArrayT group) {
    /*
    for (register int j = 0; j < group.size(); j++) {
        auto asset = group[j];
        const BWEM::Area *asset_area = the_map.GetArea(asset->get_tilepos());
        if (asset_area == nullptr || asset_area->Id() != base->get_area()->Id()) {
            base->remove_structure(asset);
            base->remove_resource_depot(asset);
        }
    }
    */
}

template <class BaseT, class UnitArrayT>
void unassign_wrong_area_worker(BWEM::Map &the_map, BaseT base, UnitArrayT group) {
    /*
    for (register int j = 0; j < group.size(); j++) {
        auto asset = group[j];
        const BWEM::Area *asset_area = the_map.GetArea(asset->get_tilepos());
        if (asset_area == nullptr || asset_area->Id() != base->get_area()->Id()) {
            base->remove_worker(asset);
        }
    }
    */
}
