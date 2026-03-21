#pragma once
#include "Definitions.h"

class NPC;

class Bullet
{
public:
    double x, y;       // world position (col, row in GL coords)
    double dirX, dirY; // normalised direction
    int    ownerTeam;  // team that fired this bullet
    bool   active;
    int    damage;

    Bullet(double startX, double startY, double targetX, double targetY,
           int team, int dmg);

    void Update();
    void Draw() const;

    // Returns true if this bullet is in the same room as an NPC and hits it
    bool CheckHit(NPC* npc);
};
