#include "Bases.h"
#include "Units.h"
#include "UnitData.h"
#include "UnitArray.h"

using namespace Basic;

namespace Basic::Bases {

namespace {

    enum WHO {
        SELF,
        ENEMY
    };

    struct BaseData {
        std::vector<BWAPI::Unit>
            larva,
            workers,
            structures,
            resource_depots;
    };

    std::vector<const BWEM::Base *> _all_bases;
    std::vector<const BWEM::Base *> _neutral_bases;
    std::vector<const BWEM::Base *> _self_bases;
    std::vector<const BWEM::Base *> _enemy_bases;

    std::vector<const BWEM::Base *> _self_just_stored;
    std::vector<const BWEM::Base *> _self_just_removed;
    std::vector<const BWEM::Base *> _enemy_just_stored;
    std::vector<const BWEM::Base *> _enemy_just_removed;

    void add_self_base(const BWEM::Base *b) {
        auto base_it = std::find(_neutral_bases.begin(), _neutral_bases.end(), b);
        if (base_it != _neutral_bases.end()) {
            _neutral_bases.erase(base_it);
            _self_bases.push_back(*base_it);
            _self_just_stored.push_back(*base_it);
            // add data
        }
        else {
            printf("Basic::Bases::add_self_base(): tried to add non-neutral base\n");
            assert(false);
        }
    }

    void add_enemy_base(const BWEM::Base *b) {
        auto base_it = std::find(_neutral_bases.begin(), _neutral_bases.end(), b);
        if (base_it != _neutral_bases.end()) {
            _neutral_bases.erase(base_it);
            _enemy_bases.push_back(*base_it);
            _enemy_just_stored.push_back(*base_it);
            // add data
        }
        else {
            printf("Basic::Bases::add_enemy_base(): tried to add non-neutral base\n");
            assert(false);
        }
    }

    std::vector<const BWEM::Base *>::iterator remove_self_base(const BWEM::Base *b) {
        auto base_it = std::find(_self_bases.begin(), _self_bases.end(), b);
        if (base_it != _self_bases.end()) {
            auto it = _self_bases.erase(base_it);
            _neutral_bases.push_back(*base_it);
            _self_just_removed.push_back(*base_it);
            return it;
        }
        else {
            printf("Basic::Bases::remove_self_base(): tried to remove non-self base\n");
            assert(false);
        }
    }

    std::vector<const BWEM::Base *>::iterator remove_enemy_base(const BWEM::Base *b) {
        auto base_it = std::find(_enemy_bases.begin(), _enemy_bases.end(), b);
        if (base_it != _enemy_bases.end()) {
            auto it = _enemy_bases.erase(base_it);
            _neutral_bases.push_back(*base_it);
            _self_just_removed.push_back(*base_it);
            return it;
        }
        else {
            printf("Basic::Bases::remove_enemy_base(): tried to remove non-enemy base\n");
            assert(false);
        }
    }
    
    void assign_new_bases() {
        const UnitArray &self_new_units = Units::self_just_stored();
        assign_by_structures(self_new_units, SELF);
        const UnitArray &self_morphed_units = Units::self_just_changed_type();
        assign_by_structures(self_morphed_units, SELF);
        const UnitArray &enemy_new_units = Units::enemy_just_stored();
        assign_by_structures(enemy_new_units, ENEMY);

        const BWAPI::Race &enemy_race = Broodwar->enemy()->getRace();
        if (enemy_race == BWAPI::Races::Zerg) {
            const UnitArray &enemy_morphed_units = Units::enemy_just_changed_type();
            assign_by_structures(enemy_morphed_units, ENEMY);
            const UnitArray &enemy_moved_units = Units::enemy_just_moved();
            assign_by_structures(enemy_moved_units, ENEMY);
        }
        else if (enemy_race == BWAPI::Races::Terran) {
            const UnitArray &enemy_moved_units = Units::enemy_just_moved();
            assign_by_structures(enemy_moved_units, ENEMY);
        }
    }

    void assign_by_structures(const UnitArray &units, WHO who) {
        for (int i = 0; i < units.size(); ++i) {
            const BWAPI::Unit &unit = units[i];
            const Units::UnitData &unit_data = Units::data(unit);
            if (unit_data.u_type == Refs::UTYPE::STRUCT) {
                const BWAPI::TilePosition structure_tilepos = unit_data.tilepos;
                if (the_map.Valid(structure_tilepos)) {
                    const BWEM::Area *structure_area = the_map.GetArea(structure_tilepos);
                    if (structure_area != nullptr) {
                        auto &area_bases = structure_area->Bases();
                        std::vector<const BWEM::Base *> potential_bases;
                        for (auto &base : area_bases) {
                            auto base_it = std::find(
                                _neutral_bases.begin(),
                                _neutral_bases.end(),
                                base
                            );
                            if (base_it != _neutral_bases.end()) {
                                potential_bases.push_back(&base);
                            }
                        }
                        if (potential_bases.size() == 1) {
                            const BWEM::Base *new_base = potential_bases[0];
                            if (who == SELF) {add_self_base(new_base);}
                            else             {add_enemy_base(new_base);}
                        }
                        else if (potential_bases.size() > 1) {
                            std::vector<double> distances;
                            for (auto &base : potential_bases) {
                                distances.push_back(
                                    structure_tilepos.getApproxDistance(base->Location())
                                );
                            }
                            auto min_dist_it = 
                                std::min_element(distances.begin(), distances.end());
                            int base_index = std::distance(distances.begin(), min_dist_it);
                            const BWEM::Base *new_base = potential_bases[base_index];
                            if (who == SELF) {add_self_base(new_base);}
                            else             {add_enemy_base(new_base);}
                        }
                    }
                }
            }
        }
    }

    void unassign_empty_bases() {
        for (auto base_it = _self_bases.begin(); base_it != _self_bases.end(); ++base_it) {
            const BWEM::Base *&base = *base_it;
            // get structure ct
            if (true) {
                remove_self_base(base);
            }
        }
        for (auto base_it = _enemy_bases.begin(); base_it != _enemy_bases.end(); ++base_it) {
            const BWEM::Base *&base = *base_it;
            // get structure ct
            if (true) {
                remove_enemy_base(base);
            }
        }
    }
}

void init() {
    const std::vector<BWEM::Area> &areas = the_map.Areas();
    for (auto &area : areas) {
        const std::vector<BWEM::Base> &bases = area.Bases();
        for (unsigned int i = 0; i < bases.size(); ++i) {
            _neutral_bases.push_back(&bases[i]);
            _all_bases.push_back(&bases[i]);
        }
    }
}

void clear_just_added_and_removed() {
    _self_just_stored.clear();
    _enemy_just_stored.clear();
    for (auto base : _self_just_removed) {
        // remove data
    }
    _self_just_removed.clear();
    for (auto base : _enemy_just_removed) {
        // remove data
    }
    _enemy_just_removed.clear();
}

bool is_self(const BWEM::Base *b) {
    for (auto &base : _self_bases) {
        if (base == b) {
            return true;
        }
    }
    return false;
}

bool is_enemy(const BWEM::Base *b) {
    for (auto &base : _enemy_bases) {
        if (base == b) {
            return true;
        }
    }
    return false;
}

bool is_neutral(const BWEM::Base *b) {
    for (auto &base : _neutral_bases) {
        if (base == b) {
            return true;
        }
    }
    return false;
}

const std::vector<const BWEM::Base *> &all_bases() {return _all_bases;}

const std::vector<const BWEM::Base *> &neutral_bases() {return _neutral_bases;}

const std::vector<const BWEM::Base *> &self_bases() {return _self_bases;}

const std::vector<const BWEM::Base *> &enemy_bases() {return _enemy_bases;}

const std::vector<const BWEM::Base *> &self_just_stored() {return _self_just_stored;}

const std::vector<const BWEM::Base *> &self_just_removed() {return _self_just_removed;}

const std::vector<const BWEM::Base *> &enemy_just_stored() {return _enemy_just_stored;}

const std::vector<const BWEM::Base *> &enemy_just_removed() {return _enemy_just_removed;}

/*
void immediately_remove_struct_from_all_bases(FUnit unit) {
    for (auto &base : self_bases) {
        for (auto &structure : base->get_structures()) {
            if (structure == unit) {
                base->remove_structure(unit);
                base->remove_resource_depot(unit);
                break;
            }
        }
    }
}
*/
}