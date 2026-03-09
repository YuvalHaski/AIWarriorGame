#include "Bullet.h"
#include "NPC.h"
#include "SecurityMap.h"
#include "glut.h"
#include <cmath>

Bullet::Bullet(double startX, double startY, double targetX, double targetY,
               int team, int dmg)
    : x(startX), y(startY), ownerTeam(team), active(true), damage(dmg)
{
    double dx = targetX - startX;
    double dy = targetY - startY;
    double len = std::sqrt(dx*dx + dy*dy);
    if (len > 0.001) { dirX = dx/len; dirY = dy/len; }
    else             { dirX = 1.0;   dirY = 0.0;   }
}

void Bullet::Update()
{
    if (!active) return;

    x += BULLET_SPEED * dirX;
    y += BULLET_SPEED * dirY;

    // Convert to grid
    int col = (int)x;
    int row = (int)y;

    if (row < 0 || row >= MSZ || col < 0 || col >= MSZ)
    { active = false; return; }

    int cell = dungeon[row][col];
    if (cell == CELL_WALL || cell == CELL_OBSTACLE)
        active = false;
}

void Bullet::Draw() const
{
    if (!active) return;
    if (ownerTeam == TEAM1)
        glColor3d(1.0, 0.5, 0.0);
    else
        glColor3d(0.2, 0.4, 1.0);

    glPointSize(3.0f);
    glBegin(GL_POINTS);
    glVertex2d(x, y);
    glEnd();
}

bool Bullet::CheckHit(NPC* npc)
{
    if (!active) return false;
    if (npc->getTeam() == ownerTeam) return false;  // no friendly fire
    if (!npc->isAlive()) return false;

    double dx = x - npc->getX();
    double dy = y - npc->getY();
    double dist = std::sqrt(dx*dx + dy*dy);
    if (dist < 0.8)
    {
        npc->takeDamage(damage);
        active = false;
        return true;
    }
    return false;
}
