#pragma once
#include <BWAPI.h>

namespace BWEB
{
    class Path {
        std::vector<BWAPI::TilePosition> tiles;
        double dist;
        bool reachable;
        BWAPI::TilePosition source, target;
    public:
        Path()
        {
            tiles ={};
            dist = 0.0;
            reachable = false;
            source = BWAPI::TilePositions::Invalid;
            target = BWAPI::TilePositions::Invalid;
        }

        /// <summary> Returns the vector of TilePositions associated with this Path. </summary>
        std::vector<BWAPI::TilePosition>& getTiles() { return tiles; }

        /// <summary> Returns the distance from the source to the target in pixels. </summary>
        double getDistance() { return dist; }

        /// <summary> Returns a check if the path was able to reach the target. </summary>
        bool isReachable() { return reachable; }

        /// <summary> Creates a path from the source to the target using JPS and collision provided by BWEB based on walkable tiles and used tiles. </summary>
        void createUnitPath(const BWAPI::Position, const BWAPI::Position);

        /// <summary> Creates a path from the source to the target using BFS and some odd collision functionality. BWEB use mostly. </summary>
        void createWallPath(const BWAPI::Position, const BWAPI::Position, bool, double);

        /// <summary> Creates a path from the source to the target using JPS, your provided walkable function, and directional vector (currently directional vector is disabled until JPS author pushes update). </summary>
        void jpsPath(const BWAPI::Position source, const BWAPI::Position target, std::function <bool(const BWAPI::TilePosition)> collision /*, std::vector<BWAPI::TilePosition> directions*/);

        /// <summary> Creates a path from the source to the target using BFS, your provided walkable function, and directional vector. </summary>
        void bfsPath(const BWAPI::Position source, const BWAPI::Position target, std::function <bool(const BWAPI::TilePosition)> collision, std::vector<BWAPI::TilePosition> directions);

        /// added by Trace Myers without permission of the creator of BWEB
        void clear() {
            tiles.clear();
            dist = 0.0;
            reachable = false;
            source = BWAPI::TilePositions::Invalid;
            target = BWAPI::TilePositions::Invalid;
        }

        /// added by Trace Myers without permission of the creator of BWEB
        double getDistanceFrom(int waypoint) { 
            double dist = 0.0;
            for (auto tile_it = tiles.begin() + waypoint; tile_it < tiles.end() - 1; ++tile_it) {
                const BWAPI::TilePosition &a = *tile_it;
                const BWAPI::TilePosition &b = *(tile_it + 1);
                dist += a.getApproxDistance(b);
            }
            return dist;
        }

        ///
        BWAPI::TilePosition getSource() { return source; }
        BWAPI::TilePosition getTarget() { return target; }
    };

    namespace Pathfinding {
        void clearCache();
    }
}
