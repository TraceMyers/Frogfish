#include "ConstructionPlanning.h"
#include "../production/BuildGraph.h"
#include "../basic/Bases.h"
#include "../test/TestMessage.h"
#include "BWEM/bwem.h"

using namespace Production;

namespace Strategy::ConstructionPlanning {

    // TODO: implement logic; just chooses whatever for now
    // (especially geysers. Just picks main base geyser)
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