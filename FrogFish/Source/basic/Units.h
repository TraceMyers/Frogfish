#pragma once
#pragma message("including Units")

#include "UnitArray.h"
#include "../utility/BWTimer.h"
#include <BWAPI.h>

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

struct UnitData {
    UnitData(BWAPI::Unit u) :
        cmd_ready(true),
        cmd_timer(),
        u_task(UTASK::IDLE),
        ID(u->getID()), 
        hp(u->getHitPoints()),
        max_hp(u->getType().maxHitPoints()),
        shields(u->getShields()),
        energy(u->getEnergy()),
        ground_weapon_cd(u->getGroundWeaponCooldown()),
        air_weapon_cd(u->getAirWeaponCooldown()),
        spell_cd(u->getSpellCooldown()),
        irradiate_timer(u->getIrradiateTimer()),
        lockdown_timer(u->getLockdownTimer()),
        plague_timer(u->getPlagueTimer()),
        stasis_timer(u->getStasisTimer()),
        transport(u->getTransport()),
        loaded_units(u->getLoadedUnits()),
        tech_researching(u->getTech()),
        upgrading(u->getUpgrade()),
        type(u->getType()), 
        init_type(u->getInitialType()),
        pos(u->getPosition()),
        tilepos(u->getTilePosition()),
        angle(u->getAngle()),
        u_type(get_utype(u)),
        missing(false),
        lifted(false)
    {
        velocity[0] = u->getVelocityX(); 
        velocity[1] = u->getVelocityY();
    }
    // self only
    bool                cmd_ready;
    BWTimer             cmd_timer;
    UTASK               u_task;
    int                 irradiate_timer;
    int                 lockdown_timer;
    int                 plague_timer;
    int                 stasis_timer;
    int                 ground_weapon_cd;
    int                 air_weapon_cd;
    int                 spell_cd;
    BWAPI::Unit         transport;
    BWAPI::Unitset      loaded_units;
    BWAPI::TechType     tech_researching;
    BWAPI::UpgradeType  upgrading;
    // both
    int                 ID;
    int                 hp;
    int                 max_hp;
    int                 shields;
    int                 energy;
    double              velocity[2] {0.0, 0.0};
    double              angle;
    bool                lifted;
    BWAPI::UnitType     type;
    BWAPI::UnitType     init_type;
    BWAPI::Position     pos;
    BWAPI::TilePosition tilepos;
    UTYPE               u_type;
    // enemy
    bool                missing;
};

}