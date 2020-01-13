#include "BaseAssets.h"
#include "../FrogFish.h"
#include "../unitdata/BaseStorage.h"
#include "../unitdata/UnitStorage.h"
#include <BWEM/bwem.h>

template <class UnitT, class BaseT>
inline void assign_asset_to_base(const UnitT asset, const BaseT base) {
    if (asset->is_struct()) {
        base->add_structure(asset);
        if (asset->get_type().isResourceDepot()) {
            base->add_resource_depot(asset);
        }
    }
    else if (asset->is_worker()) {
        base->add_worker(asset);
    }
    else if (asset->is_larva()) {
        base->add_larva(asset);
    }
}

template <class UnitT, class BaseT>
void assign_asset(const std::vector<BaseT> &bases, UnitT asset) {
    const TilePosition structure_tilepos = asset->get_tilepos();
    if (the_map.Valid(structure_tilepos)) {
        const BWEM::Area *asset_area = the_map.GetArea(structure_tilepos);
        if (asset_area != nullptr) {
            std::vector<BaseT> potential_assign_bases;
            for (unsigned int i = 0; i < bases.size(); i++) {
                BaseT base = bases[i];
                if (
                    asset_area->Id() == base->get_area()->Id()
                    && !asset->is_lifted()
                ) {
                    if (asset->is_missing()) {
                        if (asset->is_struct() && base->has_structure(asset)) {
                            base->remove_structure(asset);
                            if (asset->get_type().isResourceDepot()) {
                                base->remove_resource_depot(asset);
                            }
                        }
                    }
                    else if (
                        (asset->is_struct() && !base->has_structure(asset)) 
                        ||
                        (asset->is_worker() && !base->has_worker(asset))
                        ||
                        (asset->is_larva() && !base->has_larva(asset))
                    ) {
                        // asset->is_larva() always returns false for EUnit
                        // because I don't care about storing larva at enemy bases
                        // larva storage in EnemyBase just makes it work with this template
                        potential_assign_bases.push_back(base);
                    }
                    else if (!asset->is_struct() && base->has_structure(asset)) {
                        base->remove_structure(asset);
                        base->remove_resource_depot(asset);
                    }
                    else if (!asset->is_worker() && base->has_worker(asset)) {
                        base->remove_worker(asset);
                    }
                    else if (!asset->is_larva() && base->has_larva(asset)) {
                        base->remove_larva(asset);
                    }
                }
                else if (asset->is_struct() && base->has_structure(asset)) {
                    base->remove_structure(asset);
                    if (asset->get_type().isResourceDepot()) {
                        base->remove_resource_depot(asset);
                    }
                }
                else if (asset->is_worker() && base->has_worker(asset)) {
                    base->remove_worker(asset);
                }
                else if (asset->is_larva() && base->has_larva(asset)) {
                    base->remove_larva(asset);
                }
            }
            int potential_assign_base_ct = potential_assign_bases.size();
            if (potential_assign_base_ct == 1) {
                const auto base = potential_assign_bases[0];
                assign_asset_to_base(asset, base);
            }
            else if (potential_assign_base_ct > 1) {
                std::vector<double> distances;
                const Position &struct_pos = asset->get_pos();
                for (auto &base : potential_assign_bases) {
                    const Position &base_pos = base->get_center();
                    distances.push_back(
                        struct_pos.getApproxDistance(base_pos)
                    );
                }
                auto min_dist = std::min_element(distances.begin(), distances.end());
                int base_i = std::distance(distances.begin(), min_dist);
                const auto base = potential_assign_bases[base_i];
                assign_asset_to_base(asset, base);
            }
        }
    }
}

template <class UnitT, class BaseT>
void assign_assets(const std::map<int, UnitT> &units, const std::vector<BaseT> &bases) {
    if (bases.size() > 0) {
        std::map<int, UnitT>::const_iterator unit_it;
        for (unit_it = units.begin(); unit_it != units.end(); ++unit_it) {
            auto unit = unit_it->second;
            if (
                unit->is_worker() 
                || unit->is_struct() 
                || unit->is_larva()
                || unit->get_type() == BWAPI::UnitTypes::Zerg_Egg
            ) {
                // eggs are let through to unassign them from self base larva
                assign_asset(bases, unit);
            }
        }
    }
}

template <class UnitT, class BaseT>
void remove_dead_assets(const std::vector<UnitT> &assets, const std::vector<BaseT> &bases) {
    for (UnitT asset : assets) {
        if (asset->is_struct()) {
            if (asset->get_type().isResourceDepot()) {
                for (BaseT base : bases) {
                    if (base->has_structure(asset)) {
                        base->remove_structure(asset);
                        base->remove_resource_depot(asset);
                        break;
                    }
                }
            }
            else {
                for (BaseT base : bases) {
                    if (base->has_structure(asset)) {
                        base->remove_structure(asset);
                        break;
                    }
                }
            }
        }
        else if (asset->is_worker()) {
            for (BaseT base : bases) {
                if (base->has_worker(asset)) {
                    base->remove_worker(asset);
                    break;
                }
            }
        }
        else if (asset->is_larva()) {
            for (BaseT base : bases) {
                if (base->has_larva(asset)) {
                    base->remove_larva(asset);
                    break;
                }
            }
        }
    }
}

void assign_base_assets(BaseStorage &base_storage, UnitStorage &unit_storage) {
    const std::vector<FBase> &self_bases = base_storage.get_self_bases();
    const std::map<int, FUnit> &self_units = unit_storage.self_units();
    assign_assets(self_units, self_bases);
    const std::vector<FUnit> &self_newly_removed = unit_storage.get_self_newly_removed();
    remove_dead_assets(self_newly_removed, self_bases);
    const std::vector<EBase> &enemy_bases = base_storage.get_enemy_bases();
    const std::map<int, EUnit> &enemy_units = unit_storage.enemy_units();
    assign_assets(enemy_units, enemy_bases);
    const std::vector<EUnit> &enemy_newly_removed = unit_storage.get_enemy_newly_removed();
    remove_dead_assets(enemy_newly_removed, enemy_bases);
}
