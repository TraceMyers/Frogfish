#pragma once
#pragma message("including EnemyUnit")
#include <BWAPI.h>

using namespace BWAPI;

class EnemyUnit {

private:

    Unit bwapi_unit;
    int ID;
    int max_hp;
    int hp;
    int shields;
    int energy;
    Position pos;
    TilePosition tilepos;
    std::vector<double> velocity {0.0, 0.0};
    UnitType type;
    std::string name;
    bool missing;
    bool lifted = false;

public:

    enum ETYPE {
        WORKER,
        ARMY,
        STRUCT,
        UNKNOWN
    };

    ETYPE e_type;

    EnemyUnit(const Unit u) {
        bwapi_unit = u;
        ID = u->getID();
        update();
    }

    void update() {
        hp = bwapi_unit->getHitPoints();
        shields = bwapi_unit->getShields();
        energy = bwapi_unit->getEnergy();
        pos = bwapi_unit->getPosition();
        tilepos = bwapi_unit->getTilePosition();
        velocity[0] = bwapi_unit->getVelocityX();
        velocity[1] = bwapi_unit->getVelocityY();
        type = bwapi_unit->getType();
        max_hp = type.maxHitPoints();
        name = type.getName();
        missing = false;
        lifted = bwapi_unit->isLifted();

        if (type == UnitTypes::Unknown) {
            e_type = UNKNOWN;
        }
        else if (type.isBuilding() || type.isAddon()) {
            e_type = STRUCT;
        }
        else if (type.isWorker()) {
            e_type = WORKER;
        }
        else {
            e_type = ARMY;
        }
    }

    const Unit bwapi_u() {return bwapi_unit;}

    int get_ID() {return ID;}

    int get_max_hp() {return max_hp;}

    int get_hp() {return hp;}

    int get_shields() {return shields;}

    int get_energy() {return energy;}

    const Position get_pos() {return pos;}

    const TilePosition get_tilepos() {return tilepos;}

    const std::vector<double> &get_velocity() {return velocity;}

    const UnitType get_type() {return type;}

    const std::string &get_name() {return name;}

    void set_missing(bool value) {missing = value;}

    bool is_missing() {return missing;}

    bool is_lifted() {return lifted;}

    bool is_unknown() {return e_type == UNKNOWN;}

    bool is_struct() {return e_type == STRUCT;}

    bool is_worker() {return e_type == WORKER;}

    bool is_army() {return e_type == ARMY;}

    bool friend operator == (EnemyUnit const &e_unit1, EnemyUnit const &e_unit2) {
        return e_unit1.ID == e_unit2.ID;
    }

    // template stuff
    bool is_larva() {return false;}
};

typedef class EnemyUnit *EUnit;