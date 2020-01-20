#pragma once
#pragma message("including ConstructionStorage")

#include "BuildGraph.h"
#include "EconTracker.h"
#include "../utility/BWTimer.h"
#include "../unitdata/FrogUnit.h"
#include "../unitdata/UnitStorage.h"
#include <deque>
#include <BWAPI.h>

// TODO: After pathing is in, store path here and make a function that
// quickly approximates time left on route based on how many waypoints
// have been cleared, or just store the path here and do calculation at
// construction manager, probably

class ConstructionStorage {

private:

    static const int MAX_BUILD = 100;
    static const int _100_PERCENT = 30;
    static const int NEAR_ENOUGH = 200;

    const enum SET_TARGET_CODE { 
        SUCCESS,
        BAD_STATUS,
        NOT_FOUND
    };

    FUnit build_units[MAX_BUILD];
    BWAPI::UnitType build_types[MAX_BUILD];
    TilePosition target_nodes[MAX_BUILD];
    int build_IDs[MAX_BUILD];
    std::vector<BWAPI::Position> paths[MAX_BUILD];
    int reservation_IDs[MAX_BUILD];
    bool res_canceled[MAX_BUILD];
    int build_ct;

    void add_extractor(FUnit extractor, TilePosition target_node, int build_ID, int index);
    void advance_status(int i);
    void change_build_states(UnitStorage &unit_storage, EconTracker &econ_tracker);
    void clear_lost_and_completed();

public:

    const enum STATUS {
        NONE,
        EN_ROUTE,
        AT_SITE,
        UNDER_CONSTR,
        COMPLETED,
        LOST
    };

    STATUS status[MAX_BUILD];

    ConstructionStorage();
    void on_frame_update(UnitStorage &unit_storage, EconTracker &econ_tracker);
    STATUS get_status(int build_ID);
    FUnit get_unit(int build_ID);
    TilePosition get_target_node(int build_ID);
    SET_TARGET_CODE set_target_node(int build_ID, TilePosition target);
    void add_tracker(
        FUnit drone, 
        BWAPI::UnitType build_type, 
        TilePosition target, 
        int build_ID,
        std::vector<BWAPI::Position> _path,
        int reservation_ID
    );
    std::vector<int> lost_IDs();
    std::vector<int> completed_IDs();
    std::vector<BWAPI::UnitType> get_making_types();
};