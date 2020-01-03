#ifndef ENEMY_UNIT
#define ENEMY_UNIT

#include <BWAPI.h>

using namespace BWAPI;
using namespace Filter;

class EnemyUnit {
// Wraps the BWAPI::Unit class in order to keep track of
// enemy unit data when those units aren't seen. 

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
    }

    const BWAPI::Unit getUnitPtr() {return bwapi_unit;}

    int getID() {return ID;}

    int getInitialHitPoints() {return max_hp;}

    int getHitPoints() {return hp;}

    int getShields() {return shields;}

    int getEnergy() {return energy;}

    const BWAPI::Position& getPosition() {return pos;}

    const BWAPI::TilePosition& getTilePosition() {return tilepos;}

    const std::vector<double>& getVelocity() {return velocity;}

    const BWAPI::UnitType& getType() {return type;}

    bool friend operator == (EnemyUnit const &e_unit1, EnemyUnit const &e_unit2) {
        return e_unit1.ID == e_unit2.ID;
    }
};

#endif