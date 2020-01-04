#pragma once

#include <BWAPI.h>

using namespace BWAPI;

class FrogUnit {

private:

    Unit bwapi_unit;
    UnitType type;
    std::vector<double> velocity {0.0, 0.0};

public:

    enum FTYPE {
        EGG,
        LARVA,
        WORKER,
        ARMY,
        STRUCT
    };

    FTYPE f_type;

    FrogUnit(const Unit u) {
        bwapi_unit = u;
        update();
    }

    bool type_changed() {
        return type == bwapi_unit->getType();
    }

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

    const Unit bwapi_u() {return bwapi_unit;}

    int get_ID() {return bwapi_unit->getID();}

    int get_max_hp() {return bwapi_unit->getType().maxHitPoints();}

    int get_hp() {return bwapi_unit->getHitPoints();}

    int get_energy() {return bwapi_unit->getEnergy();}

    const Position get_pos() {return bwapi_unit->getPosition();}

    const TilePosition get_tilepos() {return bwapi_unit->getTilePosition();}

    const std::vector<double>& get_velocity() {
        velocity[0] = bwapi_unit->getVelocityX();
        velocity[1] = bwapi_unit->getVelocityY();
        return velocity;
    }

    const UnitType get_type() {return type;}

    const std::string& get_name() {return type.getName();}

    bool is_struct() {return f_type == STRUCT;}

    bool is_worker() {return f_type == WORKER;}

    bool is_army() {return f_type == ARMY;}

    bool is_larva() {return f_type == LARVA;}

    bool is_egg() {return f_type == EGG;}
};

typedef class FrogUnit *FUnit;