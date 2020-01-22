#include "TechManager.h"
#include "../unitdata/BaseStorage.h"
#include "BuildOrder.h"
#include "EconTracker.h"
#include <BWAPI.h>
#include <vector>

// TODO: make it so this doesn't crash when the building dies 

void TechManager::init_upgrades(
    BaseStorage &base_storage, 
    BuildOrder *build_order, 
    EconTracker &econ_tracker
) {
    if (
        sent_research_command != nullptr
        && sent_research_command->is_ready()
    ) {
        if ( 
            sent_research_command->bwapi_u()->isResearching()
            || sent_research_command->bwapi_u()->isUpgrading()
        ) {
            build_order->next();
            sent_research_command = nullptr;
            sent_upgrade = BWAPI::UpgradeTypes::None;
            sent_tech = BWAPI::TechTypes::None;
        }
        else if (sent_upgrade != BWAPI::UpgradeTypes::None) {
            sent_research_command->bwapi_u()->upgrade(sent_upgrade);
            sent_research_command->set_cmd_delay(2);
        }
        else if (sent_tech != BWAPI::TechTypes::None) {
            sent_research_command->bwapi_u()->research(sent_tech);
            sent_research_command->set_cmd_delay(2);
        }
        else {
            printf("TechManager.init_upgrades(): didn't store tech or upgrade to retry!\n");
        }
    }
    if (!build_order->finished()) {
        BuildItem &build_item = build_order->peek_next();
        if (build_item.build_type == BuildItem::TECH) {
            const BWAPI::TechType &tech_type = build_item.tech_type;
            int mineral_cost = tech_type.mineralPrice();
            int gas_cost = tech_type.gasPrice();
            if (
                mineral_cost <= econ_tracker.get_free_minerals()
                && gas_cost <= econ_tracker.get_free_gas()
            ) {
                BWAPI::UnitType tech_from = tech_type.whatResearches();
                for (auto &base : base_storage.get_self_bases()) {
                    for (auto &structure : base->get_structures()) {
                        if (
                            structure->get_type() == tech_from
                            && !structure->bwapi_u()->isResearching()
                            && !structure->bwapi_u()->isUpgrading()
                        ) {
                            sent_research_command = structure;
                            sent_tech = tech_type;
                            structure->bwapi_u()->research(tech_type);
                            structure->set_cmd_delay(2);
                        }
                    }
                }
            }
        }
        else if (build_item.build_type == BuildItem::UPGRADE) {
            const BWAPI::UpgradeType &upgrade_type = build_item.upgrade_type;
            int mineral_cost = upgrade_type.mineralPrice();
            int gas_cost = upgrade_type.gasPrice();
            if (
                mineral_cost <= econ_tracker.get_free_minerals()
                && gas_cost <= econ_tracker.get_free_gas()
            ) {
                BWAPI::UnitType upgrade_from = upgrade_type.whatUpgrades();
                for (auto &base : base_storage.get_self_bases()) {
                    for (auto &structure : base->get_structures()) {
                        if (
                            structure->get_type() == upgrade_from
                            && !structure->bwapi_u()->isResearching()
                            && !structure->bwapi_u()->isUpgrading()
                        ) {
                            sent_research_command = structure;
                            sent_upgrade = upgrade_type;
                            structure->bwapi_u()->upgrade(upgrade_type);
                            structure->set_cmd_delay(2);
                        }
                    }
                }
            }
        }
    }
}