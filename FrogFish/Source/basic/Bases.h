#pragma once
#pragma message("including BaseStorage")

#include "../FrogFish.h"
#include <BWEM/bwem.h>
#include <BWAPI.h>
#include <vector>

namespace Basic::Bases {
    void init();
    void add_self_base(const BWEM::Base *b);
    void remove_self_base(const FBase &f_base);
    void add_enemy_base(const BWEM::Base *b);
    void remove_enemy_base(const EBase &e_base);
    void clear_newly_assigned();
    bool base_is_self(const BWEM::Base *b);
    bool base_is_enemy(const BWEM::Base *b);
    bool base_is_neutral(const BWEM::Base *b);
    const BWEMBaseArray &get_all_bases();
    const std::vector<FBase> &get_self_bases();
    const std::vector<EBase> &get_enemy_bases();
    const BWEMBaseArray &get_neutral_bases();
    const std::vector<FBase> &get_self_newly_stored();
    const std::vector<FBase> &get_self_newly_removed();
    const std::vector<EBase> &get_enemy_newly_stored();
    const std::vector<EBase> &get_enemy_newly_removed();
    void immediately_remove_struct_from_all_bases(FUnit unit);
}