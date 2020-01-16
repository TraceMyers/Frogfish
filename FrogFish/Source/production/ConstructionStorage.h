#pragma once

#include "../unitdata/FrogUnit.h"
#include "../unitdata/UnitStorage.h"
#include "BuildGraph.h"
#include <deque>
#include <BWAPI.h>

// TODO: After pathing is in, store path here and make a function that
// quickly approximates time left on route based on how many waypoints
// have been cleared, or just store the path here and do calculation at
// construction manager, probably

class ConstructionStorage {

private:

    static const int MAX_BUILD = 100;
    static const int _100_PERCENT = 10;
    static const int NEAR_ENOUGH = 256;

    enum SET_TARGET_CODE {
        SUCCESS,
        BAD_STATUS,
        NOT_FOUND
    };

    enum STATUS {
        NONE,
        EN_ROUTE,
        AT_SITE,
        UNDER_CONSTR,
        COMPLETED,
        LOST
    };

    FUnit build_units[MAX_BUILD];
    BWAPI::UnitType build_types[MAX_BUILD];
    BNode target_nodes[MAX_BUILD];
    int build_IDs[MAX_BUILD];
    STATUS status[MAX_BUILD];
    int build_ct;


    void add_extractor(FUnit extractor, BNode target_node, int build_ID);
    void advance_status(int i);
    void change_build_states(UnitStorage &unit_storage);
    void clear_lost_and_completed();

public:

    ConstructionStorage();
    void on_frame_update(UnitStorage &unit_storage);
    STATUS get_status(int build_ID);
    FUnit get_unit(int build_ID);
    BNode get_target_node(int build_ID);
    SET_TARGET_CODE set_target_node(int build_ID, BNode target);
    void add_tracker(FUnit drone, BWAPI::UnitType build_type, BNode target, int build_ID);
    std::vector<int> lost_IDs();
    std::vector<int> completed_IDs();
};