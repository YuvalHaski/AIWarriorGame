#pragma once
#include "Definitions.h"
#include "AStar.h"
#include <vector>
#include <utility>

class NPC
{
protected:
    double x, y;            // world position (col, row in GL coords)
    int    hp;
    int    maxHp;
    int    team;
    int    npcType;         // NPC_WARRIOR / NPC_MEDIC / NPC_SUPPLY
    bool   alive;

    // Path following
    std::vector<std::pair<int,int>> path;  // (row, col) waypoints
    int pathIndex;

public:
    NPC(double startX, double startY, int hp, int team, int type);
    virtual ~NPC() {}

    virtual void DoSomeWork() = 0;
    virtual void show() = 0;

    // Plan a path from current grid cell to target grid cell
    bool PlanPathTo(int targetRow, int targetCol);

    // Move one step along planned path.
    // Returns true when path is complete (or no path planned).
    bool FollowPlannedPath(double minDist = 0.4);

    // Accessors
    double getX() const { return x; }
    double getY() const { return y; }
    int    getHp() const { return hp; }
    int    getMaxHp() const { return maxHp; }
    int    getTeam() const { return team; }
    int    getType() const { return npcType; }
    bool   isAlive() const { return alive; }

    // Returns current room id (-1 if in corridor/wall)
    int    getCurrentRoom() const;

    // Grid position
    int    getGridRow() const { return (int)y; }
    int    getGridCol() const { return (int)x; }

    void   takeDamage(int dmg);
    void   heal(int amount);

    // Path inspection (used by FSM states)
    bool   isPathEmpty() const { return path.empty(); }
    bool   isPathComplete() const { return path.empty() || pathIndex >= (int)path.size(); }
    // Last waypoint world coords (centre of last path cell)
    double getPathEndX() const
    {
        return path.empty() ? x : path.back().second + 0.5;
    }
    double getPathEndY() const
    {
        return path.empty() ? y : path.back().first + 0.5;
    }

protected:
    void drawBase(double r, double g, double b, char symbol, double sz = 1.0) const;
};
