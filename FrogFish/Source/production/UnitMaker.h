#pragma once
#pragma message("UnitMaker")

#include "MakeQueue.h"
#include "MorphQueue.h"
#include "EconTracker.h"
#include "BuildOrder.h"

class UnitMaker {

private:

    MakeQueue make_queue;
    MorphQueue morph_queue;
    BuildOrder *build_order;
    std::vector<BWTimer> overlord_making_timers;
    bool finished_init_droning = false;

    enum MODE {
        PAUSED,
        PROPORTIONAL,
        BUILD_ORDER
    } mode;
    
    void auto_push_overlord(EconTracker &econ_tracker);

    void spend_down(BaseStorage &base_storage, EconTracker &econ_Tracker);

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

    void make_units(EconTracker &econ_tracker, BaseStorage &base_storage);

    void build_order_fill_queue();
};