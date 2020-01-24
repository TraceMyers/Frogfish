#include "WorkerController.h"
#include "../unitdata/BaseStorage.h"
#include "../unitdata/UnitStorage.h"
#include "../unitdata/FrogBase.h"
#include "../unitdata/FrogUnit.h"
#include <BWEM/bwem.h>
#include <BWAPI.h>

namespace Control::Workers {

namespace {
    std::vector<FUnit> making_extractors;
}

// TODO: centralize task setting
void send_idle_workers_to_mine(BaseStorage &base_storage) {
    const std::vector<FBase> &self_bases = base_storage.get_self_bases();
    for (auto base : self_bases) {
        const std::vector<FUnit> &workers = base->get_workers();
        for (auto worker : workers) {
            if (worker->f_task == FrogUnit::FTASKS::IDLE && worker->is_ready()) {
                auto bwapi_unit = worker->bwapi_u();
                bwapi_unit->gather(bwapi_unit->getClosestUnit(Filter::IsMineralField));
                worker->f_task = FrogUnit::FTASKS::MINE_MINERALS;
                worker->set_cmd_delay(2);
            }
        }
    }
}

// TODO: 
//      - make it so an extractor always auto gets 3 even if not enough in the base 
// also... make better
void send_mineral_workers_to_gas(
    BaseStorage &base_storage,
    UnitStorage &unit_storage
) {
    for (auto &unit : unit_storage.get_self_newly_stored()) {
        printf("newly stored\n");
        if (unit->get_type() == BWAPI::UnitTypes::Zerg_Extractor) {
            making_extractors.push_back(unit);
        }
    }
    for (auto &unit : unit_storage.get_self_newly_changed_type()) {
        printf("newly changed type\n");
        if (unit->get_type() == BWAPI::UnitTypes::Zerg_Extractor) {
            making_extractors.push_back(unit);
        }
    }
    if (making_extractors.size() > 0) {
        FUnit extractor = making_extractors[0];
        if (extractor->is_ready()) {
            int workers_sent_to_gas = 0;
            for (auto &base : base_storage.get_self_bases()) {
                for (auto &structure : base->get_structures()) {
                    if (structure == extractor) {
                        for (auto &worker : base->get_workers()) {
                            if (
                                worker->f_task == FrogUnit::FTASKS::MINE_MINERALS
                                && worker->is_ready()
                            ) {
                                worker->bwapi_u()->gather(extractor->bwapi_u());
                                worker->f_task = FrogUnit::FTASKS::MINE_GAS;
                                worker->set_cmd_delay(2);
                                ++workers_sent_to_gas;
                            }
                            if (workers_sent_to_gas == 3) {
                                making_extractors.erase(making_extractors.begin());
                                return;
                            }
                        }
                    }
                }
            }
        }
    }
}

}