#pragma once

#include "../data/UnitStorage.h"
#include "../data/BaseStorage.h"
#include <BWEM/bwem.h>
#include <string>

void append_debug_text(std::string *dbg_line);
void draw_debug_text();
void draw_units(UnitStorage &us);
void draw_map(BWEM::Map &the_map);
void draw_base_info(BaseStorage &base_storage);