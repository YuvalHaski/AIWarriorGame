#pragma once
#include "Definitions.h"
#include "Bullet.h"
#include <vector>

class NPC;

// A grenade fires a burst of bullets in all directions on explosion.
class Grenade
{
public:
    double x, y;
    int    ownerTeam;
    bool   active;
    bool   exploded;
    int    fuseTimer;   // frames until explosion
    int    damage;

    static const int NUM_SHARDS = 8;
    std::vector<Bullet> shards;

    Grenade(double startX, double startY, double targetX, double targetY,
            int team, int dmg, int fuse = 20);

    void Update(std::vector<NPC*>& npcs);
    void Draw() const;

private:
    double velX, velY;
    void Explode(std::vector<NPC*>& npcs);
};
