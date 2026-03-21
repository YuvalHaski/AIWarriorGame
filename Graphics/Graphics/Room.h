#pragma once
#include "Definitions.h"

struct Room
{
    int row, col;       // top-left corner in the dungeon grid
    int width, height;  // dimensions
    int id;             // unique room id (0..numRooms-1)

    // Center of the room
    int centerRow() const { return row + height / 2; }
    int centerCol() const { return col + width  / 2; }

    // Check overlap with another room (with a 1-cell border)
    bool overlaps(const Room& other) const
    {
        return !(row + height + 1 < other.row ||
                 other.row + other.height + 1 < row ||
                 col + width  + 1 < other.col ||
                 other.col + other.width  + 1 < col);
    }


};
