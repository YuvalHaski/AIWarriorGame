#pragma once
#include <vector>
#include <utility>

// Map size
#define MSZ 100

// Cell types in dungeon[][]
#define CELL_WALL        0
#define CELL_CORRIDOR    1
#define CELL_ROOM        2
#define CELL_OBSTACLE    3   // impassable rock/pillar inside a room
#define CELL_AMMO1       4   // ammo depot (team 1 side)
#define CELL_AMMO2       5   // ammo depot (team 2 side)
#define CELL_MED1        6   // medicine depot (team 1 side)
#define CELL_MED2        7   // medicine depot (team 2 side)

// Game settings
#define NUM_ROOMS        8

// --- SPEED / TIMING ---
// Slow everything down so players can follow what is happening
#define NPC_SPEED            0.30   // grid units per frame
#define BULLET_SPEED         0.18   // grid units per frame (was 0.5)
#define GRENADE_RADIUS       5.0    // explosion radius in grid units

// --- HP / AMMO ---
#define WARRIOR_MAX_HP      200     // plenty of HP so fights last longer
#define WARRIOR_MAX_AMMO     40
#define MEDIC_MAX_HP         90
#define SUPPLY_MAX_HP        90

#define BULLET_DAMAGE        25     // must exceed heal rate (20hp/80frames) at 90-frame fire interval
#define GRENADE_DAMAGE       60

#define HEAL_AMOUNT          20
#define AMMO_REFILL          18

// --- A* / SECURITY ---
#define RISK_WEIGHT          3.0
#define SECURITY_DECAY       0.003  // slow decay so the map changes are visible

// --- COMBAT TIMING ---
#define COMBAT_TICK_INTERVAL  90    // frames between shots (was 30 - 3x slower)

// Teams
#define TEAM1  1
#define TEAM2  2

// NPC types
#define NPC_WARRIOR  0
#define NPC_MEDIC    1
#define NPC_SUPPLY   2

// Forward declarations
class NPC;
class WarriorNPC;
class MedicNPC;
class SupplyNPC;

// Global map arrays (defined in DungeonMap.cpp)
extern int    dungeon[MSZ][MSZ];
extern int    roomId[MSZ][MSZ];
extern double securityMap[MSZ][MSZ];

// Global NPC list (defined in main.cpp)
extern std::vector<NPC*> allNPCs;

// Room count (defined in DungeonMap.cpp)
extern int numRooms;

// Ammo / medicine depot grid coordinates (defined in DungeonMap.cpp)
extern int ammoDepot1Row, ammoDepot1Col;
extern int ammoDepot2Row, ammoDepot2Col;
extern int medDepot1Row,  medDepot1Col;
extern int medDepot2Row,  medDepot2Col;
