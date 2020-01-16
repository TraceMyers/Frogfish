#pragma once
#pragma message ("FrogUnit")

#include "../utility/BWTimer.h"
#include <BWAPI.h>



class FrogUnit {

private:

    BWAPI::Unit bwapi_unit;
    BWAPI::UnitType type;
    std::vector<double> velocity {0.0, 0.0};
    bool cmd_ready;
    BWTimer cmd_timer;
    int _ID;

public:

    enum FTASKS {
        IDLE,
        MINE_MINERALS,
        MINE_GAS,
        TRANSFER_BASE,
        BUILD_STRUCT,
        MORPH_UNIT,
        ATTACK,
        SCOUT
    };

    enum FTYPES {
        EGG,
        LARVA,
        WORKER,
        ARMY,
        STRUCT,
        UNASSIGNED
    };

    FTYPES f_type;
    FTASKS f_task;

    FrogUnit(const BWAPI::Unit u) : 
        bwapi_unit(u),
        cmd_ready(true),
        f_task(FTASKS::IDLE),
        f_type(FTYPES::UNASSIGNED),
        _ID(u->getID())
    {update();}

    void update() {
        type = bwapi_unit->getType();

        if (type.isBuilding()) {
            f_type = STRUCT;
        }
        else if (type.isWorker()) {
            f_type = WORKER;
        }
        else if (type.canAttack() || type.isSpellcaster() || type.isFlyer()) {
            f_type = ARMY;
        }
        else if (bwapi_unit->isMorphing()) {
            f_type = EGG;
        }
        else {
            f_type = LARVA;
        }
    }

    void update_cmd_timer() {
        cmd_timer.on_frame_update();
        if (cmd_timer.is_stopped()) {
            set_ready();
        }
    }

    const BWAPI::Unit bwapi_u() {return bwapi_unit;}

    int get_ID() {return _ID;}

    int get_max_hp() {return bwapi_unit->getType().maxHitPoints();}

    int get_hp() {return bwapi_unit->getHitPoints();}

    int get_energy() {return bwapi_unit->getEnergy();}

    const BWAPI::Position get_pos() {return bwapi_unit->getPosition();}

    const BWAPI::TilePosition get_tilepos() {return bwapi_unit->getTilePosition();}

    const std::vector<double> &get_velocity() {
        velocity[0] = bwapi_unit->getVelocityX();
        velocity[1] = bwapi_unit->getVelocityY();
        return velocity;
    }

    const BWAPI::UnitType &get_type() {return type;}

    const std::string &get_name() {return type.getName();}

    bool is_struct() {return f_type == STRUCT;}

    bool is_worker() {return f_type == WORKER;}

    bool is_army() {return f_type == ARMY;}

    bool is_larva() {return f_type == LARVA;}

    bool is_egg() {return f_type == EGG;}

    bool is_ready() {return cmd_ready;}

    bool type_changed() {return type == bwapi_unit->getType();}

    // only for use by FUnit
    void set_ready() {cmd_ready = true;}

    void set_cmd_delay(int frames) {
        cmd_ready = false;
        cmd_timer.start(0, frames);
    }

    // template stuff -----------
    bool is_lifted() {return false;}
    
    bool is_missing() {return false;}
};

typedef class FrogUnit *FUnit;
