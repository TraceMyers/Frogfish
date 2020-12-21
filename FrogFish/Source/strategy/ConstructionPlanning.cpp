#include "ConstructionPlanning.h"
#include "../production/BuildGraph.h"
#include "../basic/Bases.h"
#include "../basic/Units.h"
#include "../basic/UnitData.h"
#include "../test/TestMessage.h"
#include "BWEM/bwem.h"

using namespace Production;

namespace Strategy::ConstructionPlanning {

    namespace {

        ConstructionPlan plans[64];
        uint64_t used_plans_field = 0;
        int _plans_count = 0;

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
    int Strategy::ConstructionPlanning::make_construction_plan(const Production::BuildOrder::Item& item) {
        const std::vector<const BWEM::Base *> &bases = Basic::Bases::self_bases();
        const BWEM::Base *construction_base = nullptr;
        BWAPI::Unit builder = nullptr;
        const BWAPI::UnitType& type = item.unit_type();
        BWAPI::TilePosition build_tp;
        for (auto &base : bases) {
            auto &base_workers = Basic::Bases::workers(base);
            if (base_workers.size() > 0) {
                construction_base = base;
                for (auto &worker : base_workers) {
                    auto& worker_data = Basic::Units::data(worker);
                    if (worker_data.u_task == Basic::Refs::MINERALS && worker_data.build_status == NONE) {
                        builder = worker;
                        break;
                    }
                }
            }
            if (construction_base != nullptr && builder != nullptr) {
                break;
            }
        }
        if (construction_base == nullptr) {
            DBGMSG("ConstructionPlanning::make_construction_plan(): Couldn't find base for construction! Building type: %s", type.c_str());
            return NO_BASE;
        }
        else if (builder == nullptr) {
            DBGMSG("ConstructionPlanning::make_construction_plan(): Couldn't find builder for construction! Building type: %s", type.c_str());
            return NO_BUILDER;
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
        if (!build_tp) {
            DBGMSG("ConstructionPlanning::make_construction_plan(): Couldn't find builder for construction! Building type: %s", type.c_str());
            return NO_LOCATION;
        }

        int plan_ID = get_free_plans_index();
        plans[plan_ID].set_base(construction_base);
        plans[plan_ID].set_builder(builder);
        plans[plan_ID].set_item(item);
        plans[plan_ID].set_tilepos(build_tp);

        ++_plans_count;
        return plan_ID;
    }

    const ConstructionPlan &get_plan(int ID) {
        return plans[ID];
    }

    int find_plan(const Production::BuildOrder::Item &item) {
        uint64_t field_checker = 1;
        int i = 0;
        while(field_checker > 0) {
            if (field_checker & used_plans_field) {
                if(plans[i].get_item() == item) {
                    return i; 
                }
            }
            field_checker <<= 1;
            ++i;
        }
        return (int)NO_PLAN;
    }

    void destroy_plan(int ID) {
        uint64_t ID_field_num = 1 << ID;

        #ifdef _DEBUG
        assert(ID >= 0 && ID < 64 && (ID_field_num & used_plans_field));
        #endif

        --_plans_count;
        used_plans_field ^= ID_field_num;
    }

    void set_extractor_flag(int ID, bool value) {
        plans[ID].set_extractor_transition(value);
    }

    void replace_null_builder_with_extractor(int ID, BWAPI::Unit extractor) {
        plans[ID].set_builder(extractor);
    }

    int plans_count() {
        return _plans_count;
    }
}