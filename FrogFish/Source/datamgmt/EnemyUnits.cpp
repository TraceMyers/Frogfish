#include "EnemyUnits.h"

void flag_missing_enemy_units(UnitStorage unit_storage) {
    const std::map<int, EUnit> &enemy_units = unit_storage.enemy_units();
    std::map<int, EUnit>::const_iterator enem_it;
    for (enem_it = enemy_units.begin(); enem_it != enemy_units.end(); ++enem_it) {
        EUnit e_unit = enem_it->second;
        if (
            !(e_unit->is_missing()) 
            && !(e_unit->bwapi_u()->isVisible())
            && (Broodwar->isVisible(e_unit->get_tilepos()))
        ) {
            e_unit->set_missing(true);
        }
    }

}