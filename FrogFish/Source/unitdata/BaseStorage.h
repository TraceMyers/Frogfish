#pragma once
#pragma message("including BaseStorage")

#include "../FrogFish.h"
#include "FrogBase.h"
#include "EnemyBase.h"
#include "../unitdata/BWEMBaseArray.h"
#include <BWEM/bwem.h>
#include <BWAPI.h>
#include <vector>

using namespace BWAPI;

class BaseStorage {

private:

    BWEMBaseArray all_bases;
    BWEMBaseArray neutral_bases;
    std::vector<FBase> self_bases;
    std::vector<EBase> enemy_bases;

    std::vector<FBase> self_newly_stored;
    std::vector<FBase> self_newly_removed;
    std::vector<EBase> enemy_newly_stored;
    std::vector<EBase> enemy_newly_removed;

public:

    void init() {
        const std::vector<BWEM::Area> &areas = the_map.Areas();
        int j = 0;
        for (auto &area : areas) {
            const std::vector<BWEM::Base> &bases = area.Bases();
            for (unsigned int i = 0; i < bases.size(); ++i) {
                neutral_bases.add(&bases[i]);
                all_bases.add(&bases[i]);
                j++;
            }
        }
        printf("BaseOwnership.init_base_storage(): Added %d neutral bases\n", j);
    }

    void add_self_base(const BWEM::Base *b) {
        const BWEM::Base *j = neutral_bases.remove(b);
        if (j == nullptr)
            printf("base storage bwem base nullptr\n");
        FBase f_base = new FrogBase(b);
        self_bases.push_back(f_base);
        self_newly_stored.push_back(f_base);
    }

    void remove_self_base(const FBase &f_base) {
        self_newly_removed.push_back(f_base);
    }

    void add_enemy_base(const BWEM::Base *b) {
        neutral_bases.remove(b);
        EBase e_base = new EnemyBase(b);
        enemy_bases.push_back(e_base);
        enemy_newly_stored.push_back(e_base);
    }

    void remove_enemy_base(const EBase &e_base) {
        enemy_newly_removed.push_back(e_base);
    }

    void clear_newly_assigned() {
        self_newly_stored.clear();
        while (self_newly_removed.size() > 0) {
            FBase f_base = self_newly_removed.back();
            auto it = std::remove(self_bases.begin(), self_bases.end(), f_base);
            self_bases.erase(it, self_bases.end());
            const BWEM::Base *b = f_base->free_data();
            neutral_bases.add(b);
            self_newly_removed.pop_back();
            delete f_base;
        }
        self_newly_removed.clear();
        enemy_newly_stored.clear();
        while (enemy_newly_removed.size() > 0) {
            EBase e_base = enemy_newly_removed.back();
            auto it = std::remove(enemy_bases.begin(), enemy_bases.end(), e_base);
            enemy_bases.erase(it, enemy_bases.end());
            const BWEM::Base *b = e_base->free_data();
            neutral_bases.add(b);
            enemy_newly_removed.pop_back();
            delete e_base;
        }
        enemy_newly_removed.clear();
    }

    bool base_is_self(const BWEM::Base *b) {
        for (FBase &f_base : self_bases) {
            if (f_base->get_center() == b->Center()) {
                return true;
            }
        }
        return false;
    }

    bool base_is_enemy(const BWEM::Base *b) {
        for (EBase &e_base : enemy_bases) {
            if (e_base->get_center() == b->Center()) {
                return true;
            }
        }
        return false;
    }

    bool base_is_neutral(const BWEM::Base *b) {
        for (int i = 0; i < neutral_bases.length(); ++i) {
            if (neutral_bases[i]->Center() == b->Center()) {
                return true;
            }
        }
        return false;
    }

    const BWEMBaseArray &get_all_bases() {return all_bases;}

    const std::vector<FBase> &get_self_bases() {return self_bases;}

    const std::vector<EBase> &get_enemy_bases() {return enemy_bases;}

    const BWEMBaseArray &get_neutral_bases() {return neutral_bases;}

    const std::vector<FBase> &get_self_newly_stored() {return self_newly_stored;}

    const std::vector<FBase> &get_self_newly_removed() {return self_newly_removed;}

    const std::vector<EBase> &get_enemy_newly_stored() {return enemy_newly_stored;}

    const std::vector<EBase> &get_enemy_newly_removed() {return enemy_newly_removed;}

    void immediately_remove_struct_from_all_bases(FUnit unit) {
        for (auto &base : self_bases) {
            for (auto &structure : base->get_structures()) {
                if (structure == unit) {
                    base->remove_structure(unit);
                    base->remove_resource_depot(unit);
                    break;
                }
            }
        }
    }

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
        all_bases.free_data();
    }
};