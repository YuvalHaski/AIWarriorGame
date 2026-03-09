#pragma once
#include "Definitions.h"
#include <vector>
#include <utility>

// Find a path from (si,sj) to (ti,tj) on the dungeon grid.
// Cells with dungeon value CELL_WALL or CELL_OBSTACLE are impassable.
// The cost is weighted by the security map.
// Returns true if a path was found; outPath is filled with (row,col) pairs
// from start (exclusive) to target (inclusive).
bool FindPath(int si, int sj, int ti, int tj,
              std::vector<std::pair<int,int>>& outPath);
