#include "Grenade.h"
#include "NPC.h"
#include "SecurityMap.h"
#include "glut.h"
#include <cmath>

static const double PI = 3.14159265358979323846;

Grenade::Grenade(double startX, double startY, double targetX, double targetY,
                 int team, int dmg, int fuse)
    : x(startX), y(startY), ownerTeam(team), active(true), exploded(false),
      fuseTimer(fuse), damage(dmg)
{
    velX = (targetX - startX) / fuse;
    velY = (targetY - startY) / fuse;
}

void Grenade::Explode(std::vector<NPC*>& npcs)
{
    exploded = true;
    // Raise security in this room
    int col = (int)x;
    int row = (int)y;
    if (row >= 0 && row < MSZ && col >= 0 && col < MSZ)
    {
        int rid = roomId[row][col];
        if (rid >= 0)
            SecurityMapRaiseDanger(rid, 0.5);
    }

    // Damage any NPC within radius
    for (NPC* npc : npcs)
    {
        if (!npc->isAlive()) continue;
        if (npc->getTeam() == ownerTeam) continue;
        double dx = x - npc->getX();
        double dy = y - npc->getY();
        double dist = std::sqrt(dx*dx + dy*dy);
        if (dist <= GRENADE_RADIUS)
        {
            int scaledDmg = (int)(damage * (1.0 - dist / GRENADE_RADIUS));
            npc->takeDamage(scaledDmg);
        }
    }

    // Create shard bullets in 8 directions
    shards.clear();
    for (int i = 0; i < NUM_SHARDS; i++)
    {
        double angle = (2.0 * PI / NUM_SHARDS) * i;
        double tx = x + std::cos(angle) * 5.0;
        double ty = y + std::sin(angle) * 5.0;
        shards.push_back(Bullet(x, y, tx, ty, ownerTeam, damage / 2));
    }
}

void Grenade::Update(std::vector<NPC*>& npcs)
{
    if (!active) return;
    if (exploded)
    {
        bool anyActive = false;
        for (auto& s : shards)
        {
            s.Update();
            for (NPC* npc : npcs)
                s.CheckHit(npc);
            if (s.active) anyActive = true;
        }
        if (!anyActive) active = false;
        return;
    }

    // Move toward landing spot
    x += velX;
    y += velY;
    fuseTimer--;

    if (fuseTimer <= 0)
        Explode(npcs);
}

void Grenade::Draw() const
{
    if (!active) return;
    if (!exploded)
    {
        glColor3d(0.5, 0.5, 0.0);
        glPointSize(4.0f);
        glBegin(GL_POINTS);
        glVertex2d(x, y);
        glEnd();
    }
    else
    {
        for (const auto& s : shards)
            s.Draw();
    }
}
