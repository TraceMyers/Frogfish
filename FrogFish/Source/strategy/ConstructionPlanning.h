#pragma once
#pragma message("including ConstructionPlanning")

#include "../production/BuildOrder.h"
#include "../basic/Bases.h"
#include <stdint.h>

using namespace Production;

namespace Strategy::ConstructionPlanning {

    class ConstructionPlan {

    private:

        BWAPI::TilePosition build_tp;
        BWAPI::Unit builder;
        const BWEM::Base *build_base;
        const Production::BuildOrder::Item *_item_ptr;

    public:

        void set_base(const BWEM::Base *base) {build_base = base;}

        void set_item(const Production::BuildOrder::Item& item) {_item_ptr = &item;}

        void set_tilepos(const BWAPI::TilePosition& tp) {build_tp = tp;}

        void set_builder(const BWAPI::Unit &u) {builder = u;}

        const BWEM::Base *get_base() const {return build_base;}

        const Production::BuildOrder::Item& get_item() const {return *_item_ptr;}

        const BWAPI::TilePosition &get_tilepos() const {return build_tp;}

        const BWAPI::Unit& get_builder() const {return builder;}

        /*
        TODO:

        bool item_removed_from_build_order() {}

        bool builder_died() {}

        */
    };

    enum CONSTRUCTION_PLAN_ERRORS {
        NO_BASE = -1,
        NO_BUILDER = -2,
        NO_LOCATION = -3
    };

    int                     make_construction_plan(const Production::BuildOrder::Item& item);

    const ConstructionPlan& get_plan(int ID);

    bool                    plan_exists(const Production::BuildOrder::Item &item);

    void                    destroy_plan(int ID);
}