#include "../FrogFish.h"
#include <BWAPI.h>
#include <jps.hh>

// Leverages BWEM (include/BWEM) and a borrowed JPS implementation (jps.hh)
struct WalkGraph {

private:

    
public:

    const unsigned step = 0;
    JPS::PathVector path;
    BWAPI::WalkPosition walk_size;
    unsigned width;
    unsigned height;

    WalkGraph();

    void init();

    std::vector<BWAPI::Position> get_path(
        const BWAPI::Position &_a, 
        const BWAPI::Position &_b
    );

    double get_approx_path_time(
        std::vector<BWAPI::Position> &path, 
        BWAPI::Position start, 
        int speed
    );
};