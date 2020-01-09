#include <BWAPI.h>
#include "WorkerControl.h"
#include "../data/BaseStorage.h"
#include "../data/FrogBase.h"
#include "../data/FrogUnit.h"
#include <BWEM/bwem.h>

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
            }
        }
    }
}
