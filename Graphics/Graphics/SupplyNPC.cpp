#include "SupplyNPC.h"
#include "WarriorNPC.h"
#include "glut.h"
#include <cmath>
#include <algorithm>

SupplyNPC::SupplyNPC(double startX, double startY, int t, int depotR, int depotC)
    : NPC(startX, startY, SUPPLY_MAX_HP, t, NPC_SUPPLY),
      ammoStock(SUPPLY_MAX_STOCK), depotRow(depotR), depotCol(depotC),
      goingToDepot(false), supplyRange(2), supplyCooldown(0)
{}

void SupplyNPC::DoSomeWork()
{
    if (!alive) return;

    // Refill own ammo stock at the depot when empty
    if (ammoStock <= 0 && !goingToDepot)
    {
        PlanPathTo(depotRow, depotCol);
        goingToDepot = true;
    }

    if (goingToDepot)
    {
        bool arrived = FollowPlannedPath();
        if (arrived)
        {
            ammoStock    = SUPPLY_MAX_STOCK;
            goingToDepot = false;
        }
        return;
    }

    // Find the friendly warrior with the lowest ammo
    NPC* needsAmmo = nullptr;
    int  lowestAmmo = WARRIOR_MAX_AMMO;
    for (NPC* npc : allNPCs)
    {
        if (!npc->isAlive()) continue;
        if (npc->getTeam() != team) continue;
        if (npc->getType() != NPC_WARRIOR) continue;

        WarriorNPC* w = static_cast<WarriorNPC*>(npc);
        // Only approach warriors who are below the getAmmo threshold * 2
        if (w->getAmmo() < w->getAmmoThreshold * 2 && w->getAmmo() < lowestAmmo)
        {
            lowestAmmo = w->getAmmo();
            needsAmmo  = npc;
        }
    }

    if (needsAmmo)
    {
        double dx   = needsAmmo->getX() - x;
        double dy   = needsAmmo->getY() - y;
        double dist = std::sqrt(dx*dx + dy*dy);

        if (dist > supplyRange)
        {
            if (path.empty() || pathIndex >= (int)path.size())
                PlanPathTo(needsAmmo->getGridRow(), needsAmmo->getGridCol());
            FollowPlannedPath();
        }
        else
        {
            // In range: hand over ammo on cooldown
            if (supplyCooldown <= 0 && ammoStock > 0)
            {
                WarriorNPC* w = static_cast<WarriorNPC*>(needsAmmo);
                int give = std::min(AMMO_REFILL, ammoStock);
                w->addAmmo(give);
                ammoStock -= give;
                supplyCooldown = 80;
            }
        }
    }
    else
    {
        // Nothing to do: stay near the ammo depot
        if (path.empty() || pathIndex >= (int)path.size())
            PlanPathTo(depotRow, depotCol);
        FollowPlannedPath();
    }

    if (supplyCooldown > 0) supplyCooldown--;
}

void SupplyNPC::show()
{
    if (!alive) return;

    // Supply colour: darker/browner shade of team colour
    double cr, cg, cb;
    if (team == TEAM1) { cr=0.80; cg=0.30; cb=0.00; }
    else               { cr=0.00; cg=0.15; cb=0.75; }

    drawBase(cr, cg, cb, 'S', 0.90);

    // ---- Ammo icon: small gold squares drawn on body ----
    glColor3d(0.95, 0.80, 0.05);
    double d = 0.25;
    glBegin(GL_QUADS);
    // top-left dot
    glVertex2d(x-d-0.15, y+0.05); glVertex2d(x-0.05, y+0.05);
    glVertex2d(x-0.05, y+d+0.05); glVertex2d(x-d-0.15, y+d+0.05);
    glEnd();
    glBegin(GL_QUADS);
    // top-right dot
    glVertex2d(x+0.05, y+0.05); glVertex2d(x+d+0.15, y+0.05);
    glVertex2d(x+d+0.15, y+d+0.05); glVertex2d(x+0.05, y+d+0.05);
    glEnd();

    // ---- Ammo stock bar below body (gold/yellow) ----
    double sz    = 0.90;
    double barW  = sz * 2.0;
    double barH  = 0.25;
    double barX0 = x - sz;
    double barY0 = y - sz - barH - 0.08;
    double frac  = (double)ammoStock / (double)SUPPLY_MAX_STOCK;
    if (frac < 0) frac = 0;

    glColor3d(0.15, 0.15, 0.15);
    glBegin(GL_QUADS);
    glVertex2d(barX0,      barY0); glVertex2d(barX0+barW, barY0);
    glVertex2d(barX0+barW, barY0+barH); glVertex2d(barX0, barY0+barH);
    glEnd();

    glColor3d(0.90, 0.75, 0.0);
    glBegin(GL_QUADS);
    glVertex2d(barX0,           barY0); glVertex2d(barX0+barW*frac, barY0);
    glVertex2d(barX0+barW*frac, barY0+barH); glVertex2d(barX0, barY0+barH);
    glEnd();

    glColor3d(0,0,0);
    glBegin(GL_LINE_LOOP);
    glVertex2d(barX0,      barY0); glVertex2d(barX0+barW, barY0);
    glVertex2d(barX0+barW, barY0+barH); glVertex2d(barX0, barY0+barH);
    glEnd();

    // Label
    glColor3d(0.90, 0.90, 0.90);
    glRasterPos2d(x - sz, y + sz + 0.55);
    const char* lbl = "Supply";
    for (const char* p = lbl; *p; p++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *p);
}
