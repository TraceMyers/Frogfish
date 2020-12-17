#include "Workers.h"
#include "../basic/Bases.h"
#include "../basic/Units.h"
#include "../basic/References.h"
#include <BWEM/bwem.h>
#include <BWAPI.h>

namespace Control::Workers {

// namespace {
//     std::vector<FUnit> making_extractors;
// }

void send_idle_workers_to_mine() {
    const std::vector<const BWEM::Base *>& bases = Basic::Bases::self_bases();
    for (auto base : bases) {
        const std::vector<BWAPI::Unit> &workers = Basic::Bases::workers(base);
        int counter = 0;
        for (auto worker : workers) {
            auto &unit_data = Basic::Units::data(worker);
            if (unit_data.u_task == Basic::Refs::IDLE && unit_data.cmd_ready) {
                worker->gather(worker->getClosestUnit(Filter::IsMineralField));
                Basic::Units::set_utask(worker, Basic::Refs::MINERALS);
                Basic::Units::set_cmd_delay(worker, 2);
            }
        }
    }
}

// void send_mineral_workers_to_gas(
//     BaseStorage &base_storage,
//     UnitStorage &unit_storage
// ) {
//     for (auto &unit : unit_storage.get_self_newly_stored()) {
//         printf("newly stored\n");
//         if (unit->get_type() == BWAPI::UnitTypes::Zerg_Extractor) {
//             making_extractors.push_back(unit);
//         }
//     }
//     for (auto &unit : unit_storage.get_self_newly_changed_type()) {
//         printf("newly changed type\n");
//         if (unit->get_type() == BWAPI::UnitTypes::Zerg_Extractor) {
//             making_extractors.push_back(unit);
//         }
//     }
//     if (making_extractors.size() > 0) {
//         FUnit extractor = making_extractors[0];
//         if (extractor->is_ready()) {
//             int workers_sent_to_gas = 0;
//             for (auto &base : base_storage.get_self_bases()) {
//                 for (auto &structure : base->get_structures()) {
//                     if (structure == extractor) {
//                         for (auto &worker : base->get_workers()) {
//                             if (
//                                 worker->f_task == FrogUnit::FTASKS::MINE_MINERALS
//                                 && worker->is_ready()
//                             ) {
//                                 worker->bwapi_u()->gather(extractor->bwapi_u());
//                                 worker->f_task = FrogUnit::FTASKS::MINE_GAS;
//                                 worker->set_cmd_delay(2);
//                                 ++workers_sent_to_gas;
//                             }
//                             if (workers_sent_to_gas == 3) {
//                                 making_extractors.erase(making_extractors.begin());
//                                 return;
//                             }
//                         }
//                     }
//                 }
//             }
//         }
//     }
// }

}