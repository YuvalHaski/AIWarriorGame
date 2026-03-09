#include "MedicNPC.h"
#include "WarriorNPC.h"
#include "glut.h"
#include <cmath>
#include <algorithm>
#include <string>

static const int MEDIC_MAX_STOCK = 12;

MedicNPC::MedicNPC(double startX, double startY, int t, int depotR, int depotC)
    : NPC(startX, startY, MEDIC_MAX_HP, t, NPC_MEDIC),
      healCooldown(0), medicineStock(MEDIC_MAX_STOCK),
      depotRow(depotR), depotCol(depotC),
      goingToDepot(false), healRange(2)
{}

void MedicNPC::DoSomeWork()
{
    if (!alive) return;

    // Go refill medicine stock if empty
    if (medicineStock <= 0 && !goingToDepot)
    {
        PlanPathTo(depotRow, depotCol);
        goingToDepot = true;
    }

    if (goingToDepot)
    {
        bool arrived = FollowPlannedPath();
        if (arrived)
        {
            medicineStock = MEDIC_MAX_STOCK;
            goingToDepot  = false;
        }
        return;
    }

    // Find the friendly warrior most in need of healing
    NPC* wounded = nullptr;
    int  worstHp  = WARRIOR_MAX_HP;
    for (NPC* npc : allNPCs)
    {
        if (!npc->isAlive()) continue;
        if (npc->getTeam() != team) continue;
        if (npc->getType() != NPC_WARRIOR) continue;
        if (npc->getHp() < worstHp)
        {
            worstHp  = npc->getHp();
            wounded  = npc;
        }
    }

    if (wounded)
    {
        double dx = wounded->getX() - x;
        double dy = wounded->getY() - y;
        double dist = std::sqrt(dx*dx + dy*dy);

        if (dist > healRange)
        {
            // Move toward the wounded warrior (replan if path is done)
            if (path.empty() || pathIndex >= (int)path.size())
                PlanPathTo(wounded->getGridRow(), wounded->getGridCol());
            FollowPlannedPath();
        }
        else
        {
            // In range: heal on cooldown
            if (healCooldown <= 0 && medicineStock > 0)
            {
                wounded->heal(HEAL_AMOUNT);
                medicineStock--;
                healCooldown = 80;
            }
        }
    }
    else
    {
        // No one needs healing: stay near the medicine depot
        if (path.empty() || pathIndex >= (int)path.size())
            PlanPathTo(depotRow, depotCol);
        FollowPlannedPath();
    }

    if (healCooldown > 0) healCooldown--;
}

void MedicNPC::show()
{
    if (!alive) return;

    // Medic colour: lighter/more yellow version of team colour
    double cr, cg, cb;
    if (team == TEAM1) { cr=1.00; cg=0.75; cb=0.25; }
    else               { cr=0.25; cg=0.65; cb=1.00; }

    drawBase(cr, cg, cb, 'M', 0.90);

    // ---- White '+' cross drawn on the body to clearly show it's a medic ----
    glColor3d(1.0, 1.0, 1.0);
    glLineWidth(2.5f);
    glBegin(GL_LINES);
    glVertex2d(x,      y - 0.55);  glVertex2d(x,      y + 0.55);  // vertical
    glVertex2d(x-0.55, y);         glVertex2d(x+0.55, y);          // horizontal
    glEnd();
    glLineWidth(1.0f);

    // ---- Medicine stock bar (below body, cyan) ----
    double sz    = 0.90;
    double barW  = sz * 2.0;
    double barH  = 0.25;
    double barX0 = x - sz;
    double barY0 = y - sz - barH - 0.08;
    double frac  = (double)medicineStock / (double)MEDIC_MAX_STOCK;
    if (frac < 0) frac = 0;

    glColor3d(0.15, 0.15, 0.15);
    glBegin(GL_QUADS);
    glVertex2d(barX0,      barY0); glVertex2d(barX0+barW, barY0);
    glVertex2d(barX0+barW, barY0+barH); glVertex2d(barX0, barY0+barH);
    glEnd();

    glColor3d(0.0, 0.85, 0.85);
    glBegin(GL_QUADS);
    glVertex2d(barX0,           barY0); glVertex2d(barX0+barW*frac, barY0);
    glVertex2d(barX0+barW*frac, barY0+barH); glVertex2d(barX0, barY0+barH);
    glEnd();

    glColor3d(0,0,0);
    glBegin(GL_LINE_LOOP);
    glVertex2d(barX0,      barY0); glVertex2d(barX0+barW, barY0);
    glVertex2d(barX0+barW, barY0+barH); glVertex2d(barX0, barY0+barH);
    glEnd();

    // Label "MED" above NPC
    glColor3d(0.90, 0.90, 0.90);
    glRasterPos2d(x - sz, y + sz + 0.55);
    const char* lbl = "Medic";
    for (const char* p = lbl; *p; p++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *p);
}
