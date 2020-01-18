#pragma once
#pragma message("including WorkerControl")

#include "../unitdata/BaseStorage.h"

namespace WorkerControl {
    void send_idle_workers_to_mine(BaseStorage &base_storage);
}