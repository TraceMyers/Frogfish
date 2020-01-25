#pragma once
#pragma message("including Units")

#include "UnitArray.h"
#include "UnitData.h"
#include "References.h"
#include "../utility/BWTimer.h"
#include <BWAPI.h>

using namespace Basic::Refs;

namespace Basic::Units {

void            on_frame_update();
void            queue_store(const BWAPI::Unit u);
void            queue_remove(const BWAPI::Unit u);
void            clear_newly_stored_and_removed();
void            set_utask(BWAPI::Unit u, UTASK task); // only for self units
const UnitArray &self_units();
const UnitArray &enemy_units();
const UnitData  &data(BWAPI::Unit u);
const UnitArray &self_just_stored();
const UnitArray &self_just_destroyed();
const UnitArray &self_just_changed_type();
const UnitArray &enemy_just_stored();
const UnitArray &enemy_just_destroyed();
const UnitArray &enemy_just_changed_type();
const UnitArray &enemy_just_moved();

}