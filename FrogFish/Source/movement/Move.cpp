#include "Move.h"
#include "../basic/UnitData.h"
#include "../basic/Units.h"
#include "../utility/FrogMath.h"
#include "../FrogFish.h"
#include <BWAPI.h>
#include <BWEB/BWEB.h>
#include <math.h>


// TODO: air unit move/attack move

namespace Movement::Move {

    namespace {

        std::vector<std::vector<BWAPI::Unit>>   groups;
        std::vector<float>                      cohesion_radii;
        std::vector<STATUS>                     statuses;
        std::vector<BWEB::Path>                 paths;
        std::vector<int>                        waypoints;
        std::vector<int>                        distance_remaining;
        std::vector<BWAPI::Position>            previous_positions;
        std::vector<float>                      average_speeds;
        std::vector<BWTimer>                    cohesion_wait_timers;
        std::vector<BWTimer>                    waypoint_reachable_timers;
        std::vector<int>                        unused_group_IDs;
        BWEB::Path                              check_waypoint_path;

        const int MOVE_CMD_DELAY_FRAMES = 2,
                  COHESION_WAIT_MAX_FRAMES = 48, // wait frames before timeout = this - 1
                  WAYPOINT_REACHABLE_CHECK_INTERVAL = 5;
        

        BWAPI::TilePosition nearest_valid_tilepos(BWAPI::TilePosition dest) {
            const int MAX_SEARCH_STEPS = 30;
            if (dest.isValid()) {
                if (BWEB::Map::isWalkable(dest)) {
                    return dest;
                }
                int x = dest.x;
                int y = dest.y;
                DIRECTIONS dir = UP;
                DIRECTIONS remember_dir;
                int inner_steps_bound = 1;
                int inner_steps = 0;
                int outer_steps_bound = 2;
                int outer_steps = 0;
                BWAPI::TilePosition search_tp = BWAPI::TilePositions::None;
                for (int i = 0; i < MAX_SEARCH_STEPS; ++i) {
                    remember_dir = dir;
                    switch(dir) {
                        case UP:
                            --search_tp.y;
                            dir = RIGHT;
                            break;
                        case RIGHT:
                            ++search_tp.x;
                            dir = DOWN;
                            break;
                        case DOWN:
                            ++search_tp.y;
                            dir = LEFT;
                            break;
                        case LEFT:
                            --search_tp.x;
                            dir = UP;
                    }
                    if (BWEB::Map::isWalkable(search_tp)) {break;}
                    ++inner_steps;
                    if (inner_steps == inner_steps_bound) {
                        ++outer_steps;
                        if (outer_steps == outer_steps_bound) {
                            ++inner_steps_bound;
                            inner_steps = 0;
                            outer_steps = 0;
                        }
                    }
                    else {dir = remember_dir;}
                }
                return search_tp;
            }
            else {return BWAPI::TilePositions::None;}
        }

        BWAPI::Position nearest_valid_pos(BWAPI::Position dest) {
            return BWAPI::Position(nearest_valid_tilepos(BWAPI::TilePosition(dest)));
        }

        float calculate_cohesion_radius(std::vector<BWAPI::Unit> &units, float cohesion_factor) {
            int group_width = 0, group_height = 0;
            for (auto &unit : units) {
                group_width += unit->getRight() - unit->getLeft();
                group_height += unit->getBottom() - unit->getTop();
            }
            int units_size = units.size();
            float average_width = (float)group_width / units_size,
                  average_height = (float)group_height / units_size,
                  max_avg_dim = (average_width > average_height ? average_width : average_height),
                  space_buffer_factor = 1.2f,
                  radius = 
                    (ceilf(sqrtf(units_size)) * sqrtf(2 * max_avg_dim * max_avg_dim * space_buffer_factor)) 
                    / 2.0f;
            return radius * cohesion_factor;
        }

        void move_unit(BWAPI::Unit unit, const BWAPI::Position &target_pos, const STATUS &status) {
            if (status == ATTACK_MOVING) {unit->patrol(target_pos);}
            else                         {unit->move(target_pos);}
            Basic::Units::set_cmd_delay(unit, MOVE_CMD_DELAY_FRAMES);
        }

        // Only for attack-moving units that never engage. Since attack-moving units are moved
        // with patrol, to avoid a patrol at the very end, the units are told to stop.
        void end_attack_move(std::vector<BWAPI::Unit> group, const BWAPI::Position &target_pos) {
            for (auto &unit : group) {
                BWAPI::UnitType &type = unit->getType();
                // TODO: probably others. Units that can't attack-move
                if (
                    type == BWAPI::UnitTypes::Zerg_Lurker
                    || type == BWAPI::UnitTypes::Zerg_Defiler
                ) {
                    unit->move(target_pos);
                }
                else {unit->attack(target_pos);}
            }
        }

        // TODO: account for unit size
        void move_group(
            std::vector<BWAPI::Unit> &group,
            BWEB::Path &path,
            const float &radius,
            STATUS &status,
            int &waypoint,
            int &dist_remain,
            BWAPI::Position &prev_pos,
            BWTimer &wait_timer,
            BWTimer &waypoint_check_timer
        ) {
            std::vector<BWAPI::TilePosition> &path_tiles = path.getTiles();
            BWAPI::TilePosition target_tilepos = path_tiles[waypoint]; 
            BWAPI::Position target_pos (target_tilepos.x * 32, target_tilepos.y * 32);
            BWAPI::TilePosition avg_tilepos = Utility::FrogMath::average_tileposition(group);
            BWAPI::Position avg_pos (avg_tilepos.x * 32, avg_tilepos.y * 32);
            dist_remain -= avg_pos.getApproxDistance(prev_pos);
            prev_pos = avg_pos;
            if (waypoint_check_timer.is_stopped()) {
                check_waypoint_path.createUnitPath(nearest_valid_pos(avg_pos), target_pos);
                if (!check_waypoint_path.isReachable()) {
					printf("unreachable waypoint\n");
                    status = UNREACHABLE_WAYPOINT;
                    return;
                }
                waypoint_check_timer.start(0, WAYPOINT_REACHABLE_CHECK_INTERVAL);
            }

            int distance_to_target = avg_tilepos.getApproxDistance(target_tilepos);
            if (distance_to_target <= radius) {
                bool wait_for_cohesion = false;
                for (auto unit_it = group.begin(); unit_it != group.end(); ++unit_it) {
                    auto &unit = *unit_it;
                    if (!unit->exists()) {
                        unit_it = group.erase(unit_it);
                        if (unit_it == group.end()) {break;}
                        else                        {continue;}
                    }
                    float distance_outside_radius = avg_pos.getApproxDistance(unit->getPosition()) - radius;
                    if (distance_outside_radius > 0) {
                        wait_for_cohesion = true;
                    }
                    auto &unit_data = Basic::Units::data(unit);
                    if (unit_data.cmd_ready) {move_unit(unit, target_pos, status);}
                }
                if (!wait_for_cohesion) {
                    wait_timer.start(0, 0);
                    ++waypoint;
					printf("waypoint: %d, waypoints_count: %d\n", path_tiles.size());
                    if (waypoint == path_tiles.size()) {
						printf("HERE HERE\n");
                        end_attack_move(group, target_pos);
                        status = DESTINATION;
                    }
                }
                else {
                    int wait_frames_left = wait_timer.get_frames_left();
                    if (wait_frames_left == 1)      {status = COHESION_WAIT_TIMEOUT;}
                    else if (wait_frames_left == 0) {wait_timer.start(0, COHESION_WAIT_MAX_FRAMES);}
                }
            }
            else for (auto unit_it = group.begin(); unit_it != group.end(); ++unit_it) {
                auto &unit = *unit_it;
                if (!unit->exists()) {
                    unit_it = group.erase(unit_it);
                    continue;
                }
                auto &unit_data = Basic::Units::data(unit);
                if (unit_data.cmd_ready) {move_unit(unit, target_pos, status);}
            }
            if (group.size() == 0) {status = KILLED;}
        }
    }

    int move(
        std::vector<BWAPI::Unit> &units, 
        BWAPI::TilePosition dest,
        bool attack,
        bool wait,
        float cohesion_factor
    ) {
        dest = nearest_valid_tilepos(dest);
        if (dest == BWAPI::TilePositions::None) {
            return INVALID_DEST;
        }
        BWAPI::TilePosition src = Utility::FrogMath::average_tileposition(units); 
        src = nearest_valid_tilepos(src);
        if (src == BWAPI::TilePositions::None) {
            return INVALID_SRC;
        }
        
        int group_ID;
        int unused_group_IDs_size = unused_group_IDs.size();
        if (unused_group_IDs_size > 0) {
            group_ID = unused_group_IDs[unused_group_IDs_size - 1];
            paths[group_ID].createUnitPath(BWAPI::Position(src), BWAPI::Position(dest));
            if (!paths[group_ID].isReachable()) {return UNREACHABLE_DEST;}
            if (attack) {
                if (wait) {statuses[group_ID] = WAITING_ATTACK;}
                else      {statuses[group_ID] = ATTACK_MOVING;}
            }
            else {
                if (wait) {statuses[group_ID] = WAITING_MOVE;}
                else      {statuses[group_ID] = MOVING;}
            }
            groups[group_ID] = units;
            waypoints[group_ID] = 0;
            distance_remaining[group_ID] = paths[group_ID].getDistance();
            previous_positions[group_ID] = BWAPI::Position(src);
            average_speeds[group_ID] = Utility::FrogMath::average_speed(units);
            cohesion_radii[group_ID] = calculate_cohesion_radius(units, cohesion_factor);
            cohesion_wait_timers[group_ID].start(0, 0);
            waypoint_reachable_timers[group_ID].start(0, WAYPOINT_REACHABLE_CHECK_INTERVAL);
            unused_group_IDs.erase(unused_group_IDs.end() - 1);
        }
        else {
            BWEB::Path p = BWEB::Path();
            p.createUnitPath(BWAPI::Position(src), BWAPI::Position(dest));
            if (!p.isReachable()) {return UNREACHABLE_DEST;}
            if (attack) {
                if (wait) {statuses.push_back(WAITING_ATTACK);}
                else      {statuses.push_back(ATTACK_MOVING);}
            }
            else {
                if (wait) {statuses.push_back(WAITING_MOVE);}
                else      {statuses.push_back(MOVING);}
            }
            group_ID = groups.size();
            paths.push_back(p);
            groups.push_back(units);
            waypoints.push_back(0);
            distance_remaining.push_back(p.getDistance());
            previous_positions.push_back(BWAPI::Position(src));
            average_speeds.push_back(Utility::FrogMath::average_speed(units));
            cohesion_radii.push_back(calculate_cohesion_radius(units, cohesion_factor));
            cohesion_wait_timers.push_back(BWTimer());
            BWTimer waypoint_timer = BWTimer();
            waypoint_timer.start(0, WAYPOINT_REACHABLE_CHECK_INTERVAL);
            waypoint_reachable_timers.push_back(waypoint_timer);
        }
        return group_ID;
    }

    int move(const BWAPI::Unit u, BWAPI::TilePosition dest, bool attack, bool wait) {
        return move(std::vector<BWAPI::Unit> {u}, dest, attack, wait, C_MAX);
    }

    STATUS get_status(int group_ID) {
        return (0 <= group_ID && group_ID < groups.size() ? statuses[group_ID] : BAD_GROUP_ID);
    }

    void start(int group_ID) {
        if (0 <= group_ID && group_ID < groups.size()) {
            if      (statuses[group_ID] == WAITING_ATTACK)  {statuses[group_ID] = ATTACK_MOVING;}
            else if (statuses[group_ID] == WAITING_MOVE)    {statuses[group_ID] = MOVING;}
        }
    }

    int remaining_distance(int group_ID) {
        return distance_remaining[group_ID];
    }

    int remaining_frames(int group_ID) {
        return (int)round(distance_remaining[group_ID] / average_speeds[group_ID]);
    }

    bool remove(int group_ID) {
        if (group_ID < groups.size() && statuses[group_ID] != UNUSED_GROUP) {
            groups[group_ID].clear();
            statuses[group_ID] = UNUSED_GROUP;
            unused_group_IDs.push_back(group_ID);
            return true;
        }
        return false;
    }

    void on_frame_update() {
        auto group_it = groups.begin();
        auto cohesion_radii_it = cohesion_radii.begin();
        auto status_it = statuses.begin();
        auto path_it = paths.begin();
        auto waypoint_it = waypoints.begin();
        auto distance_remaining_it = distance_remaining.begin();
        auto previous_pos_it = previous_positions.begin();
        auto cohesion_wait_timer_it = cohesion_wait_timers.begin();
        auto waypoint_reachable_timer_it = waypoint_reachable_timers.begin();

        while (group_it != groups.end()) {
            STATUS &status = *status_it;
            if (status == MOVING || status == ATTACK_MOVING) {
                auto &group = *group_it;
                auto &path = *path_it;
                auto &radius = *cohesion_radii_it;
                auto &waypoint = *waypoint_it;
                auto &dist_remain = *distance_remaining_it;
                auto &prev_pos = *previous_pos_it;
                auto &wait_timer = *cohesion_wait_timer_it;
                auto &waypoint_check_timer = *waypoint_reachable_timer_it;
                move_group(
                    group, 
                    path, 
                    radius, 
                    status, 
                    waypoint, 
                    dist_remain, 
                    prev_pos,
                    wait_timer, 
                    waypoint_check_timer
                );
                wait_timer.on_frame_update();
                waypoint_check_timer.on_frame_update();
            }
            ++group_it;
            ++cohesion_radii_it;
            ++status_it;
            ++path_it;
            ++waypoint_it;
            ++distance_remaining_it = distance_remaining.begin();
            ++previous_pos_it = previous_positions.begin();
            ++cohesion_wait_timer_it;
            ++waypoint_reachable_timer_it;
        }
    }
}