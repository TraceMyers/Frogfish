#pragma once
#pragma message("including UnitStorage")

#include "EnemyUnit.h"
#include "FrogUnit.h"
#include <map>
#include <iostream>
#include <vector>

namespace UnitData::UnitStorage {
    void on_frame_update();
    void queue_store(const BWAPI::Unit u);
    void queue_remove(const BWAPI::Unit u);
    void clear_newly_assigned();
    const std::vector<BWAPI::Unit> &get_self_units();
    const std::vector<BWAPI::Unit> &get_enemy_units();
    const std::vector<FUnit> &get_self_newly_stored();
    const std::vector<FUnit> &get_self_newly_removed();
    const std::vector<FUnit> &get_self_newly_changed_type();
    const std::vector<EUnit> &get_enemy_newly_stored();
    const std::vector<EUnit> &get_enemy_newly_removed();
    const std::vector<EUnit> &get_enemy_newly_changed_type();
    const std::vector<EUnit> &get_enemy_newly_changed_pos();
}