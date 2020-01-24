#pragma once
#pragma message("including WorkerController")

#include "../unitdata/BaseStorage.h"
#include "../unitdata/UnitStorage.h"

namespace Control::Workers {
    void send_idle_workers_to_mine(BaseStorage &base_storage);

    void send_mineral_workers_to_gas(
        BaseStorage &base_storage,
        UnitStorage &unit_storage
    );
}