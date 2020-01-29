#pragma once
#pragma message("including BuildGraph")

#include "../FrogFish.h"
#include "../basic/Units.h"
#include "../basic/Bases.h"
#include "../utility/BWTimer.h"
#include <BWEM/bwem.h>
#include <BWAPI.h>

////////////////////////////////////////+/////////////////////////////////////////
///                                BuildGraph                                   //
//////////////////////////////////////////////////////////////////////////////////

namespace BuildGraph {

void                       init();
void                       on_frame_update();
bool                       buildable(TilePosition &tilepos);
void                       clear();
void                       free_data();

}

