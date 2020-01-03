#ifndef ENEMY_UNIT
#define ENEMY_UNIT

#include <BWAPI.h>

using namespace BWAPI;
using namespace Filter;

class EnemyUnit {

private:

    BWAPI::Unit bwapi_unit;
    int ID;
    int max_hp;
    int hp;
    int shields;
    int energy;
    BWAPI::Position pos;
    BWAPI::TilePosition tilepos;
    std::vector<double> velocity {0.0, 0.0};
    BWAPI::UnitType type;
    std::string name;

public:

    enum ETYPE {
        WORKER,
        ARMY,
        STRUCT,
        UNKNOWN
    };

    ETYPE e_type;

    EnemyUnit(const BWAPI::Unit u) {
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

        if (type == BWAPI::UnitTypes::Unknown) {
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

    const BWAPI::Unit bwapi_u() {return bwapi_unit;}

    int get_ID() {return ID;}

    int get_max_hp() {return max_hp;}

    int get_hp() {return hp;}

    int get_shields() {return shields;}

    int get_energy() {return energy;}

    const BWAPI::Position get_pos() {return pos;}

    const BWAPI::TilePosition get_tilepos() {return tilepos;}

    const std::vector<double>& get_velocity() {return velocity;}

    const BWAPI::UnitType get_type() {return type;}

    const std::string& get_name() {return name;}

    bool is_unknown() {return e_type == UNKNOWN;}

    bool is_struct() {return e_type == STRUCT;}

    bool is_worker() {return e_type == WORKER;}

    bool is_army() {return e_type == ARMY;}

    bool friend operator == (EnemyUnit const &e_unit1, EnemyUnit const &e_unit2) {
        return e_unit1.ID == e_unit2.ID;
    }
};

typedef class EnemyUnit *EUnit;

#endif