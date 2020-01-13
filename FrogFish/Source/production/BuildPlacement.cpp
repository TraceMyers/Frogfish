#include "BuildPlacement.h"
#include "BuildGraph.h"
#include "../unitdata/FrogBase.h"
#include "../unitdata/BaseStorage.h"
#include <vector>

FBase self_bases[BuildPlacement::BASELEN] {nullptr};
BuildGraph self_graphs[BuildPlacement::BASELEN];

void BuildPlacement::on_frame_update(BaseStorage &base_storage) {
    auto &self_removed_bases = base_storage.get_self_newly_removed();
    auto &self_stored_bases = base_storage.get_self_newly_stored();
    for (auto base : self_removed_bases) {
        for (unsigned int i = 0; i < BuildPlacement::BASELEN; ++i) {
            if (self_bases[i] == base) {
                self_bases[i] = nullptr;
                self_graphs[i].clear();
            }
        }
    }
    for (auto base : self_stored_bases) {
        for (unsigned int i = 0; i < BuildPlacement::BASELEN; ++i) {
            if (self_bases[i] == nullptr) {
                self_bases[i] = base;
                self_graphs[i].init(base);
                break;
            }
            
        }
    }
    for (unsigned int i = 0; i < BuildPlacement::BASELEN; ++i) {
        if (self_graphs[i].get_base() != nullptr) {
            self_graphs[i].on_frame_update();
        }
    }
}

BuildGraph *BuildPlacement::get_graphs() {
    return self_graphs;
}
