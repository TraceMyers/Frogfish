#include "BuildPlacement.h"
#include "BuildGraph.h"
#include "../pathing/PathFinding.h"
#include "../unitdata/FrogBase.h"
#include "../unitdata/BaseStorage.h"
#include "../unitdata/UnitStorage.h"
#include "BWEM/bwem.h"
#include <vector>

// TODO: in init, search around geyser, map each geyser to a pathable
// tileposition

namespace BuildPlacement {
    FBase self_bases[MAX_BASES] {nullptr};
    BuildGraph self_graphs[MAX_BASES];
    const BWEM::Base *bases[MAX_BASES] {nullptr};
    const BWEM::Base *closest[MAX_BASES][MAX_BASES] {nullptr};
    int base_ct = 0;

    const BWEM::Base *temp_save = nullptr;
}

void BuildPlacement::init_base_ground_distances(BaseStorage &base_storage) {
    auto &all_bases = base_storage.get_all_bases();
    const int speed = BWAPI::UnitTypes::Zerg_Drone.topSpeed();
    base_ct = all_bases.length();
    for (int i = 0; i < base_ct; ++i) {
        const BWEM::Base *from_base = all_bases[i];        
        bases[i] = from_base;
        int distances[MAX_BASES] {-1};
        for (int j = 0; j < base_ct; ++j) {
            if (i != j) {
                BWEB::Path path;  
                path.createUnitPath(from_base->Center(), all_bases[j]->Center());                
                distances[j] = path.getDistance();
            }
            else {
                distances[j] = 0;
            }
        }
        int prev_min = -1;
        for (int j = 0; j < base_ct; ++j) {
            int min_dist = 999999;
            int min_index = 0;
            for (int k = 0; k < base_ct; ++k) {
                if (distances[k] < min_dist && distances[k] > prev_min) {
                    min_dist = distances[k];
                    min_index = k;
                }
            }
            closest[i][j] = all_bases[min_index];
            prev_min = min_dist;
        }
    }
}

void BuildPlacement::on_frame_update(BaseStorage &base_storage) {
    auto &self_removed_bases = base_storage.get_self_newly_removed();
    auto &self_stored_bases = base_storage.get_self_newly_stored();
    for (auto base : self_removed_bases) {
        for (unsigned int i = 0; i < BuildPlacement::MAX_BASES; ++i) {
            if (self_bases[i] == base) {
                self_bases[i] = nullptr;
                self_graphs[i].clear();
            }
        }
    }
    for (auto base : self_stored_bases) {
        for (unsigned int i = 0; i < BuildPlacement::MAX_BASES; ++i) {
            if (self_bases[i] == nullptr) {
                self_bases[i] = base;
                self_graphs[i].init(base);
                break;
            }
            
        }
    }
    for (unsigned int i = 0; i < BuildPlacement::MAX_BASES; ++i) {
        if (self_bases[i] != nullptr) {
            self_graphs[i].on_frame_update();
        }
    }
    if (temp_save != nullptr) {
        for (int i = 0; i < base_ct; ++i) {
            if (bases[i] == temp_save) {
                for (int j = 0; j < base_ct; ++j) {
                    BWAPI::Position base_loc(closest[i][j]->Location());
                    Broodwar->drawTextMap(base_loc, "%d", j);
                }
                break;
            }
        }
    }
}

TilePosition BuildPlacement::find_any_node_for_placement(FBase base, int width, int height) {
    for (int i = 0; i < BuildPlacement::MAX_BASES; ++i) {
        if (self_bases[i] == base) {
            const auto &bnodes = self_graphs[i].get_nodes();
            for (auto &node : bnodes) {
                auto &node_build_dims = node->get_buildable_dimensions();
                if (
                    node_build_dims[0] >= width 
                    && node_build_dims[1] >= height
                    && !node->blocks_mining()
                ) {
                    return node->get_tilepos();
                }
            }
        }
    }
    return TilePosition(-1, -1);
}

// TODO: need to replace this with a function that calls on some logic depending on
// current game state to determine where next base goes.
// this just returns the nearest
TilePosition BuildPlacement::find_expansion_tilepos(BaseStorage &base_storage, FBase base) {
    temp_save = base->get_bwem_base();
    for (int i = 0; i < base_ct; ++i) {
        const BWEM::Base *b = bases[i];
        if (b->Center() == base->get_center()) {
            for (int j = 1; j < base_ct; ++j) {
                const BWEM::Base *next_closest = closest[i][j];
                if (base_storage.base_is_neutral(next_closest)) {
                    return next_closest->Location();
                }
            }
        }
    }
    return TilePosition(-1, -1);
} 

TilePosition BuildPlacement::find_node_for_tech_placement(FBase base, int width, int height) {
    return BWAPI::TilePosition(0, 0);
}

TilePosition BuildPlacement::get_base_geyser_tilepos(FBase base) {
    for (const BWEM::Geyser *geyser : base->get_geysers()) {
        if (
            geyser->Unit()->exists() 
            && geyser->Unit()->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser 
        ) {
            return geyser->TopLeft();
        }
    }
    return TilePosition(-1 , -1);
}

BuildGraph *BuildPlacement::get_graphs() {
    return self_graphs;
} 

bool BuildPlacement::graph_ready(FBase base) {
    for (int i = 0; i < BuildPlacement::MAX_BASES; ++i) {
        if (self_bases[i] == base) {
            return self_graphs[i].graph_ready;
        }
    }
	return false;
}