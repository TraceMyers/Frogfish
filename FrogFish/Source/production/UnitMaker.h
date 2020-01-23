#pragma once
#pragma message("including UnitMaker")

#include "MakeQueue.h"
#include "EconTracker.h"
#include "BuildOrder.h"
#include "../unitdata/TechStorage.h"

class UnitMaker {

private:
    MakeQueue make_queue;
    BuildOrder *build_order;
    std::vector<BWTimer> overlord_making_timers;
    bool finished_init_droning = false;

    enum MODE {
        PAUSED,
        PROPORTIONAL,
        BUILD_ORDER
    } mode;
    
    void auto_push_overlord(EconTracker &econ_tracker);

    void spend_down(
        BaseStorage &base_storage, 
        EconTracker &econ_tracker,
        TechStorage &tech_storage
    );

public:

    UnitMaker();

    void set_mode(MODE _mode);

    void on_frame_update();

    void take_proportional_order(
        BaseStorage &base_storage,
        const std::vector<double> &make_proportions,
        const std::vector<bool> &make_priorities
    );

    void take_build_order(BuildOrder *_build_order);

    void make_units(
        EconTracker &econ_tracker, 
        BaseStorage &base_storage,
        TechStorage &tech_storage
    );

    void build_order_fill_queue();

    MakeQueue &get_make_queue();
};