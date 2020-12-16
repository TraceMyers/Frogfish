#include "TestMove.h"
#include "../basic/Units.h"
#include "../movement/Move.h"
#include "../Frogfish.h"
#include "BWEM/bwem.h"

namespace Test::Move {

    namespace {

        // move_unit_to_map_center()
        bool moved_unit_a = false;
        int moved_unit_ID = -1;
        bool move_unit_to_map_center_complete = false;

    }

    void move_unit_to_map_center() {
        if (!moved_unit_a) {
            auto &self_units = Basic::Units::self_units();
            const BWAPI::TilePosition &size = the_map.Size();
            BWAPI::TilePosition half_size (size.x / 2, size.y / 2);

            for (int i = 0; i < self_units.size(); ++i) {
                const BWAPI::Unit& u = self_units[i];
                if (u->getType() == BWAPI::UnitTypes::Zerg_Drone) {
                    moved_unit_ID = Movement::Move::move(u, half_size);
                    moved_unit_a = true;
                    break;
                }
            }
        }
        else if (!move_unit_to_map_center_complete) {
            printf(
                "waypoint: %d, waypoints_count: %d\n", 
                Movement::Move::get_waypoint(moved_unit_ID),
                Movement::Move::get_path_tiles(moved_unit_ID).size()
            );
            bool reached_dest = (Movement::Move::get_status(moved_unit_ID) == Movement::Move::DESTINATION);
            if (reached_dest) {
                printf("Moved unit reached destination!\n");
                bool remove_success = Movement::Move::remove(moved_unit_ID);
                if (remove_success) {
                    printf("SUCCESS: Successfully removed move group!\n");
                }
                else {
                    printf("ERROR: Did not successfully remove move group!\n");
                }

                move_unit_to_map_center_complete = true;
            }
        }
    }
}