#pragma once
#include "Definitions.h"
#include "Room.h"
#include <vector>

// Generates the dungeon: rooms, corridors, obstacles, depots.
// Fills dungeon[][], roomId[][], securityMap[][].
// Sets ammo/med depot coordinates and numRooms.
void GenerateDungeon();

// Draw the dungeon using OpenGL
void DrawDungeon();

// Access the list of rooms
extern std::vector<Room> rooms;
