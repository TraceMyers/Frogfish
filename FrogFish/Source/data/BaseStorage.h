#pragma once

#include "../utility/storage/FBArray.h"
#include "../utility/storage/EBArray.h"
#include "../utility/storage/BWEMBArray.h"
#include <BWEM/bwem.h>
#include <BWAPI.h>

using namespace BWAPI;

class BaseStorage {

private:

    FBArray self_bases;
    EBArray enemy_bases;
    BWEMBArray neutral_bases;

    FBArray self_newly_stored;
    FBArray self_newly_removed;
    EBArray enemy_newly_stored;
    EBArray enemy_newly_removed;

public:

    // only for initialization @ BaseOwnership.init_base_storage()
    void add_neutral_base(const BWEM::Base *b) {
        neutral_bases.add(b);
    }

    void add_self_base(const BWEM::Base *b) {
        neutral_bases.remove(b);
        FBase f_base = new FrogBase(b);
        self_bases.add(f_base);
        self_newly_stored.add(f_base);
    }

    void remove_self_base(const FBase &f_base) {
        self_bases.remove(f_base);
        const BWEM::Base *b = f_base->free_data();
        neutral_bases.add(b);
        self_newly_removed.add(f_base);
    }

    void add_enemy_base(const BWEM::Base *b) {
        neutral_bases.remove(b);
        EBase e_base = new EnemyBase(b);
        enemy_bases.add(e_base);
        enemy_newly_stored.add(e_base);
    }

    void remove_enemy_base(const EBase &e_base) {
        enemy_bases.remove(e_base);
        const BWEM::Base *b = e_base->free_data();
        neutral_bases.add(b);
        enemy_newly_removed.add(e_base);
    }

    void clear_newly_assigned() {
        self_newly_stored.clear();
        while (self_newly_removed.length() > 0) {
            FBase f_base = self_newly_removed.remove_at(0);
            delete f_base;
        }
        self_newly_removed.clear();
        enemy_newly_stored.clear();
        while (enemy_newly_removed.length() > 0) {
            EBase e_base = enemy_newly_removed.remove_at(0);
        }
        enemy_newly_removed.clear();
    }

    const FBArray &get_self_bases() {return self_bases;}

    const EBArray &get_enemy_bases() {return enemy_bases;}

    const BWEMBArray &get_neutral_bases() {return neutral_bases;}

    const FBArray &get_self_newly_stored() {return self_newly_stored;}

    const FBArray &get_self_newly_removed() {return self_newly_removed;}

    const EBArray &get_enemy_newly_stored() {return enemy_newly_stored;}

    const EBArray &get_enemy_newly_removed() {return enemy_newly_removed;}

    // Only called by FrogFish::onEnd()
    void free_data() {
        while (self_bases.length() > 0) {
            FBase f_base = self_bases.remove_at(0);
            delete f_base;
        }
        while (enemy_bases.length() > 0) {
            EBase e_base = enemy_bases.remove_at(0);
            delete e_base;
        }
        self_bases.free_data();
        enemy_bases.free_data();
        neutral_bases.free_data();
        self_newly_stored.free_data();
        self_newly_removed.free_data();
        enemy_newly_stored.free_data();
        enemy_newly_removed.free_data();
    }
};