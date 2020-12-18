#include "ConstructionPlanning.h"
#include "../production/BuildGraph.h"
#include "../basic/Bases.h"
#include "../test/TestMessage.h"
#include "BWEM/bwem.h"

using namespace Production;

namespace Strategy::ConstructionPlanning {

    namespace {

        ConstructionPlan plans[64];
        uint64_t used_plans_field = 0;

        int get_free_plans_index() {
            uint64_t bit_checker = 1;
            int i = 0;
            while(bit_checker > 0) {
                if (!(used_plans_field & bit_checker)) {
                    used_plans_field ^= bit_checker;
                    break;
                }
                bit_checker <<= 1;
                i += 1;
            }

            #ifdef _DEBUG
            assert(i < 64);
            #endif

            return i;
        }
    }

    // TODO: use logic to make decisions
    int make_construction_plan(const Production::BuildOrder::Item& item) {

        const std::vector<const BWEM::Base *> &bases = Basic::Bases::self_bases();
        const BWEM::Base *construction_base = nullptr;
        const BWAPI::Unit *builder_ptr = nullptr;
        const BWAPI::UnitType& type = item.unit_type();
        BWAPI::TilePosition build_tp;
        for (auto &base : bases) {
            auto &base_workers = Basic::Bases::workers(base);
            if (base_workers.size() > 0) {
                construction_base = base;
                break;
            }
        }
        if (!construction_base) {
            DBGMSG("Couldn't find base for construction! Building type: %s", type.c_str());
            return NO_BASE;
        }
        if (type == BWAPI::UnitTypes::Zerg_Extractor) {
            build_tp = Production::BuildGraph::get_geyser_tilepos(construction_base);
        }
        else {
            build_tp = Production::BuildGraph::get_build_tilepos(
                construction_base, 
                type.tileWidth(), 
                type.tileHeight()
            );
        }

        int plan_ID = get_free_plans_index();

        return plan_ID;
    }

    const ConstructionPlan &get_plan(int ID) {
        return plans[ID];
    }

    void destroy_plan(int ID) {
        int ID_field_num = 1 << ID;

        #ifdef _DEBUG
        assert(ID >= 0 && ID < 64 && ID_field_num & used_plans_field);
        #endif

        used_plans_field ^= ID_field_num;
    }

    BWAPI::TilePosition get_construction_location(const BuildOrder::Item& item) {
        const std::vector<const BWEM::Base *> &bases = Basic::Bases::self_bases();
        const BWEM::Base *construction_base = nullptr;
        const BWAPI::UnitType& type = item.unit_type();
        for (auto &base : bases) {
            if (Basic::Bases::workers(base).size() > 0) {
                construction_base = base;
                break;
            }
        }
        if (!construction_base) {
            DBGMSG("Couldn't find base for construction! Building type: %s", type.c_str());
            return BWAPI::TilePositions::None;
        }
        if (type == BWAPI::UnitTypes::Zerg_Extractor) {
            return Production::BuildGraph::get_geyser_tilepos(construction_base);
        }
        return Production::BuildGraph::get_build_tilepos(
            construction_base, 
            type.tileWidth(), 
            type.tileHeight()
        );
    }
}