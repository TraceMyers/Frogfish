#include "ConstructionStorage.h"
#include "../unitdata/FrogUnit.h"
#include "../unitdata/UnitStorage.h"
#include "BuildGraph.h"
#include <deque>
#include <BWAPI.h>

// TODO: After pathing is in, store path here and make a function that
// quickly approximates time left on route based on how many waypoints
// have been cleared, or just store the path here and do calculation at
// construction manager, probably

ConstructionStorage::ConstructionStorage() :
    build_units(),
    build_types(),
    target_nodes(),
    build_IDs(),
    status(),
    build_ct(0)
{} 

void ConstructionStorage::on_frame_update(UnitStorage &unit_storage) {
    clear_lost_and_completed();
    change_build_states(unit_storage);
}

ConstructionStorage::STATUS ConstructionStorage::get_status(int build_ID) {
    for (int i = 0; i < build_ct; ++i) {
        if (build_ID == build_IDs[i]) {
            return status[i];
        }
    }
    return NONE;
}

FUnit ConstructionStorage::get_unit(int build_ID) {
    for (int i = 0; i < build_ct; ++i) {
        if (build_ID == build_IDs[i]) {
            return build_units[i];
        }
    }
    return nullptr;
}

BNode ConstructionStorage::get_target_node(int build_ID) {
    for (int i = 0; i < build_ct; ++i) {
        if (build_ID == build_IDs[i]) {
            return target_nodes[i];
        }
    }
    return nullptr;
}

ConstructionStorage::SET_TARGET_CODE ConstructionStorage::set_target_node(
    int build_ID, 
    BNode target
) {
    for (int i = 0; i < build_ct; ++i) {
        if (build_ID == build_IDs[i]) {
            if (status[i] == EN_ROUTE || status[i] == AT_SITE) {
                target_nodes[i] = target;
                BWAPI::Position target_pos = target->get_pos();
                if (build_units[i]->get_pos().getApproxDistance(target_pos) > NEAR_ENOUGH) {
                    status[i] = EN_ROUTE;
                }
                else {
                    status[i] = AT_SITE;
                }
                return SUCCESS;
            }
            else {
                return BAD_STATUS;
            }
        }
    }
    return NOT_FOUND;
}

void ConstructionStorage::add_tracker(FUnit drone, BWAPI::UnitType build_type, BNode target, int build_ID) {
    if (build_ct < MAX_BUILD) {
        build_units[build_ct] = drone;
        build_types[build_ct] = build_type;
        target_nodes[build_ct] = target;
        build_IDs[build_ct] = build_ID;
        status[build_ct] = EN_ROUTE;
        ++build_ct;
    }
    else {
        printf("ConstructionStorage.add_order() build_ct == MAX_BUILD\n");
    }
}

void ConstructionStorage::add_extractor(FUnit extractor, BNode target_node, int build_ID) {
    if (build_ct < MAX_BUILD) {
        build_units[build_ct] = extractor;
        build_types[build_ct] = BWAPI::UnitTypes::Zerg_Extractor;
        target_nodes[build_ct] = target_node;
        build_IDs[build_ct] = build_ID;
        status[build_ct] = UNDER_CONSTR;
        ++build_ct;
        extractor->set_cmd_delay(BWAPI::UnitTypes::Zerg_Extractor.buildTime() + _100_PERCENT);
    }
    else {
        printf("ConstructionStorage.add_extractor() build_ct == MAX_BUILD\n");
    }
}

void ConstructionStorage::advance_status(int i) {
    switch(status[i]) {
        case EN_ROUTE:
            status[i] = AT_SITE;
        break;
        case AT_SITE:
            status[i] = UNDER_CONSTR;
            build_units[i]->set_cmd_delay(build_types[i].buildTime() + _100_PERCENT);
        break;
        case UNDER_CONSTR:
            status[i] = COMPLETED;
        break;
        default:
            printf("ConstructionStorage.advance_status() got bad code %d\n", status[i]);
    }
}

void ConstructionStorage::change_build_states(UnitStorage &unit_storage) {
    std::vector<FUnit> extractors_started;
    auto &newly_stored_units = unit_storage.get_self_newly_stored();
    for (auto &unit : newly_stored_units) {
        if (unit->get_type() == BWAPI::UnitTypes::Zerg_Extractor) {
            extractors_started.push_back(unit);
        }
    }
    auto &newly_removed_units = unit_storage.get_self_newly_removed();
    for (int i = 0; i < build_ct; ++i) {
        for (auto &unit : newly_removed_units) {
            FUnit build_unit;
            if (unit->get_ID() == build_unit->get_ID()) {
                if (
                    unit->get_type() == UnitTypes::Zerg_Drone
                    && build_types[i] == UnitTypes::Zerg_Extractor
                    && extractors_started.size() > 0
                ) {
                    add_extractor(extractors_started[0], target_nodes[i], build_IDs[i]);
                    extractors_started.erase(extractors_started.begin());
                }
                status[i] = LOST;
            }
        }
        BWAPI::Position target_pos = target_nodes[i]->get_pos();
        if (
            (status[i] == EN_ROUTE
            && build_units[i]->get_pos().getApproxDistance(target_pos) <= NEAR_ENOUGH)
            ||
            (status[i] == AT_SITE
            && build_units[i]->get_type() == build_types[i])
            ||
            (status[i] == UNDER_CONSTR
            && build_units[i]->is_ready())
        ) {
            advance_status(i);
        }
    }
}

void ConstructionStorage::clear_lost_and_completed() {
    for (int i = 0; i < build_ct; ++i) {
        if (status[i] == COMPLETED || status[i] == LOST) {
            --build_ct;
            if (build_ct > 0) {
                build_units[i] = build_units[build_ct];
                build_types[i] = build_types[build_ct];
                target_nodes[i] = target_nodes[build_ct];
                build_IDs[i] = build_IDs[build_ct];
                status[i] = status[build_ct];
            }
        }
    }
}

std::vector<int> ConstructionStorage::lost_IDs() {
    std::vector<int> trash_IDs;
    for (int i = 0; i < build_ct; ++i) {
        if (status[i] == LOST) {
            trash_IDs.push_back(build_IDs[i]);
        }
    }
    return trash_IDs;
}

std::vector<int> ConstructionStorage::completed_IDs() {
    std::vector<int> success_IDs;
    for (int i = 0; i < build_ct; ++i) {
        if (status[i] == COMPLETED) {
            success_IDs.push_back(build_IDs[i]);
        }
    }
    return success_IDs;
}