#include "level.h"

Level* Level::current_level_ = nullptr;
Level* Level::prev_level_ = nullptr;
std::vector<Level*> Level::all_levels_;