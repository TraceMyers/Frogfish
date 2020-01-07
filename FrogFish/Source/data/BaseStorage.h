#pragma once

#include "FrogBase.h"
#include "EnemyBase.h"
#include "../utility/BWEMBaseArray.h"
#include <BWEM/bwem.h>
#include <BWAPI.h>
#include <vector>

using namespace BWAPI;

class BaseStorage {

private:

    BWEMBaseArray neutral_bases;
    std::vector<FBase> self_bases;
    std::vector<EBase> enemy_bases;

    std::vector<FBase> self_newly_stored;
    std::vector<FBase> self_newly_removed;
    std::vector<EBase> enemy_newly_stored;
    std::vector<EBase> enemy_newly_removed;

public:

    // only for initialization @ BaseOwnership.init_base_storage()
    void add_neutral_base(const BWEM::Base *b) {
        neutral_bases.add(b);
    }

    void add_self_base(const BWEM::Base *b) {
        neutral_bases.remove(b);
        FBase f_base = new FrogBase(b);
        self_bases.push_back(f_base);
        self_newly_stored.push_back(f_base);
    }

    void remove_self_base(const FBase &f_base) {
        std::remove(self_bases.begin(), self_bases.end(), f_base);
        const BWEM::Base *b = f_base->free_data();
        neutral_bases.add(b);
        self_newly_removed.push_back(f_base);
    }

    void add_enemy_base(const BWEM::Base *b) {
        neutral_bases.remove(b);
        EBase e_base = new EnemyBase(b);
        enemy_bases.push_back(e_base);
        enemy_newly_stored.push_back(e_base);
    }

    void remove_enemy_base(const EBase &e_base) {
        std::remove(enemy_bases.begin(), enemy_bases.end(), e_base);
        const BWEM::Base *b = e_base->free_data();
        neutral_bases.add(b);
        enemy_newly_removed.push_back(e_base);
    }

    void clear_newly_assigned() {
        self_newly_stored.clear();
        while (self_newly_removed.size() > 0) {
            FBase f_base = self_newly_removed.back();
            self_newly_removed.pop_back();
            delete f_base;
        }
        self_newly_removed.clear();
        enemy_newly_stored.clear();
        while (enemy_newly_removed.size() > 0) {
            EBase e_base = enemy_newly_removed.back();
            enemy_newly_removed.pop_back();
            delete e_base;
        }
        enemy_newly_removed.clear();
    }

    const std::vector<FBase> &get_self_bases() {return self_bases;}

    const std::vector<EBase> &get_enemy_bases() {return enemy_bases;}

    const BWEMBaseArray &get_neutral_bases() {return neutral_bases;}

    const std::vector<FBase> &get_self_newly_stored() {return self_newly_stored;}

    const std::vector<FBase> &get_self_newly_removed() {return self_newly_removed;}

    const std::vector<EBase> &get_enemy_newly_stored() {return enemy_newly_stored;}

    const std::vector<EBase> &get_enemy_newly_removed() {return enemy_newly_removed;}

    // Only called by FrogFish::onEnd()
    void free_data() {
        while (self_bases.size() > 0) {
            FBase f_base = self_bases.back();
            self_bases.pop_back();
            delete f_base;
        }
        while (enemy_bases.size() > 0) {
            EBase e_base = enemy_bases.back();
            enemy_bases.pop_back();
            delete e_base;
        }
        neutral_bases.free_data();
    }
};