#pragma once

#include <BWAPI.h>
#include "../utility/BWTimer.h"

using namespace BWAPI;

class FrogUnit {

private:

    Unit bwapi_unit;
    UnitType type;
    std::vector<double> velocity {0.0, 0.0};
    bool cmd_ready;
    BWTimer<FrogUnit *> cmd_timer;

public:

    enum FTASKS {
        IDLE,
        MINE_MINERALS,
        MINE_GAS,
        TRANSFER_BASE,
        MORPH_STRUCT,
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

    FrogUnit(const Unit u) : 
        bwapi_unit(u),
        cmd_ready(true),
        f_task(FTASKS::IDLE),
        f_type(FTYPES::UNASSIGNED)
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
    }

    const Unit bwapi_u() {return bwapi_unit;}

    int get_ID() {return bwapi_unit->getID();}

    int get_max_hp() {return bwapi_unit->getType().maxHitPoints();}

    int get_hp() {return bwapi_unit->getHitPoints();}

    int get_energy() {return bwapi_unit->getEnergy();}

    const Position get_pos() {return bwapi_unit->getPosition();}

    const TilePosition get_tilepos() {return bwapi_unit->getTilePosition();}

    const std::vector<double> &get_velocity() {
        velocity[0] = bwapi_unit->getVelocityX();
        velocity[1] = bwapi_unit->getVelocityY();
        return velocity;
    }

    const UnitType &get_type() {return type;}

    const std::string &get_name() {return type.getName();}

    bool is_struct() {return f_type == STRUCT;}

    bool is_worker() {return f_type == WORKER;}

    bool is_army() {return f_type == ARMY;}

    bool is_larva() {return f_type == LARVA;}

    bool is_egg() {return f_type == EGG;}

    bool is_ready() {return cmd_ready;}

    bool type_changed() {return type == bwapi_unit->getType();}

    // only for use by FUnit
    static void set_ready(FrogUnit *f_unit) {f_unit->cmd_ready = true;}

    void set_cmd_delay(int frames) {
        cmd_ready = false;
        static void (*func)(FrogUnit *f_unit) = &set_ready;
        cmd_timer.start(this, func, 0, frames);
    }

    // template stuff -----------
    bool is_lifted() {return false;}
    
    bool is_missing() {return false;}
};

typedef class FrogUnit *FUnit;
