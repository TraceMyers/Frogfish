#include "MakeUnits.h"
#include "MakeQueue.h"
#include "../data/EconTracker.h"
#include "../data/BaseStorage.h"
#include <BWAPI.h>
#include <vector>

#define EXTRA_DELAY_FRAMES 50
// for now, just make anywhere
// probably very temp
void spend_down(
    BaseStorage &base_storage, 
    MakeQueue &make_queue, 
    EconTracker &econ_tracker
) {
    bool still_spending = true;
    for (auto &base : base_storage.get_self_bases()) {
        for (auto &larva : base->get_larva()) {
            if (make_queue.order_filled()) {
                still_spending = false;
                break;
            }
            BWAPI::UnitType next_unit = make_queue.front();
            if (next_unit.mineralPrice() <= econ_tracker.get_free_minerals()
                && next_unit.gasPrice() <= econ_tracker.get_free_gas()
            ) {
                larva->set_cmd_delay(next_unit.buildTime() + EXTRA_DELAY_FRAMES);
                larva->bwapi_u()->morph(next_unit);
                make_queue.pop();
            }
            else {
                still_spending = false;
                break;
            }
        }
        if (!still_spending) {
            break;
        }
    }
}