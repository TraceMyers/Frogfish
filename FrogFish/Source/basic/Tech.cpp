#include "Tech.h"
#include "EnemyBase.h"
#include "Bases.h"
#include "References.h"
#include "../production/BuildOrder.h"
#include <BWAPI.h>

// TODO: implement enemy as well

void TechStorage::on_frame_update(BaseStorage &base_storage) {
    std::vector<BWAPI::UnitType> struct_types;
	struct_types.push_back(BWAPI::UnitTypes::Zerg_Larva);

    for (auto &base : base_storage.get_self_bases()) {
        for (auto &structure : base->get_structures()) {
            bool recorded = false;
			if (structure->is_ready()) {
				const BWAPI::UnitType &this_type = structure->get_type();
				for (auto &type : struct_types) {
					if (this_type == type) {
						recorded = true;
					}
				}
				if (!recorded) {
					struct_types.push_back(this_type);
				}
			}
        }
    }

    _self_can_make.clear();
    for (int i = 0; i < TypeAndName::ZERG_TYPE_CT; ++i) {
        const BWAPI::UnitType &this_type = TypeAndName::ZERG_TYPES[i];
        BWAPI::UnitType immediate_requirement = 
            ZergRequirements::ZERG_UNIT_REQ[this_type];

        bool requirement_met = false;
        for (auto & structure : struct_types) {
            if (structure == immediate_requirement || immediate_requirement == BWAPI::UnitTypes::None) {
                requirement_met = true;
                break;
            }
        }
        if (requirement_met) {
            if (this_type == BWAPI::UnitTypes::Zerg_Lurker) {
                bool found_lurker_aspect = false;
                for (auto &tech : self_techs) {
                    if (tech == BWAPI::TechTypes::Lurker_Aspect) {
                        found_lurker_aspect = true;
                        break;
                    }
                }
                if (found_lurker_aspect) {
                    _self_can_make[this_type] = true;
                }
                else {
                    _self_can_make[this_type] = false;
                }
            }
            else {
                _self_can_make[this_type] = true;
            }
        }
        else {
            _self_can_make[this_type] = false;
        }
    }
}

bool TechStorage::self_can_make(BWAPI::UnitType &ut) {
    return _self_can_make[ut];
}