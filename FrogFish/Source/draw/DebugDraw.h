#pragma once

#include "../data/UnitStorage.h"
#include <string>

void append_debug_text(std::string *dbg_line);
void draw_debug_text();
void draw_units(UnitStorage &us);