#include "WarriorNPC.h"
#include "SearchEnemyState.h"
#include "glut.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <string>

WarriorNPC::WarriorNPC(double startX, double startY, int t)
    : NPC(startX, startY, WARRIOR_MAX_HP, t, NPC_WARRIOR),
      ammo(WARRIOR_MAX_AMMO), maxAmmo(WARRIOR_MAX_AMMO),
      attackCooldown(0)
{
    // Random personality: aggression in [0.5, 1.0]
    aggression = 0.5 + (rand() % 51) / 100.0;

    // Thresholds are scaled by personality:
    //   Aggressive warriors (high aggression) have lower flee/heal/ammo thresholds
    //   Cautious warriors   (low  aggression) give up sooner
    fleeHpThreshold   = (int)(WARRIOR_MAX_HP   * (0.12 + (1.0 - aggression) * 0.15));
    fleeAmmoThreshold = (int)(WARRIOR_MAX_AMMO  * (0.05 + (1.0 - aggression) * 0.05));
    getAmmoThreshold  = (int)(WARRIOR_MAX_AMMO  * (0.28 + (1.0 - aggression) * 0.20));
    getHealThreshold  = (int)(WARRIOR_MAX_HP    * (0.30 + (1.0 - aggression) * 0.25));

    pCurrentState = new SearchEnemyState();
    pCurrentState->OnEnter(this);
}

WarriorNPC::~WarriorNPC()
{
    delete pCurrentState;
}

void WarriorNPC::ChangeState(State* newState)
{
    if (pCurrentState)
    {
        pCurrentState->OnExit(this);
        delete pCurrentState;
    }
    pCurrentState = newState;
    if (pCurrentState)
        pCurrentState->OnEnter(this);
}

NPC* WarriorNPC::FindNearestEnemy() const
{
    NPC* nearest = nullptr;
    double bestDist = 1e9;
    for (NPC* npc : allNPCs)
    {
        if (!npc->isAlive()) continue;
        if (npc->getTeam() == team) continue;
        double dx = npc->getX() - x;
        double dy = npc->getY() - y;
        double d  = dx*dx + dy*dy;
        if (d < bestDist) { bestDist = d; nearest = npc; }
    }
    return nearest;
}

NPC* WarriorNPC::FindFriendlyMedic() const
{
    for (NPC* npc : allNPCs)
    {
        if (!npc->isAlive()) continue;
        if (npc->getTeam() != team) continue;
        if (npc->getType() == NPC_MEDIC) return npc;
    }
    return nullptr;
}

NPC* WarriorNPC::FindFriendlySupply() const
{
    for (NPC* npc : allNPCs)
    {
        if (!npc->isAlive()) continue;
        if (npc->getTeam() != team) continue;
        if (npc->getType() == NPC_SUPPLY) return npc;
    }
    return nullptr;
}

void WarriorNPC::FireBullet(NPC* target)
{
    if (ammo <= 0) return;
    ammo--;
    bullets.push_back(Bullet(x, y, target->getX(), target->getY(), team, BULLET_DAMAGE));
}

void WarriorNPC::FireGrenade(NPC* target)
{
    if (ammo < 3) return;
    ammo -= 3;
    grenades.push_back(Grenade(x, y, target->getX(), target->getY(), team, GRENADE_DAMAGE, 30));
}

void WarriorNPC::DoSomeWork()
{
    if (!alive) return;

    // Update active bullets
    for (auto& b : bullets)
    {
        b.Update();
        for (NPC* npc : allNPCs)
            b.CheckHit(npc);
    }
    bullets.erase(
        std::remove_if(bullets.begin(), bullets.end(),
                       [](const Bullet& b){ return !b.active; }),
        bullets.end());

    // Update active grenades
    for (auto& g : grenades)
        g.Update(allNPCs);
    grenades.erase(
        std::remove_if(grenades.begin(), grenades.end(),
                       [](const Grenade& g){ return !g.active; }),
        grenades.end());

    // FSM drives everything (movement + state transitions)
    if (pCurrentState)
        pCurrentState->Transition(this);
}

void WarriorNPC::show()
{
    if (!alive) return;

    // Draw bullets and grenades underneath the NPC
    for (const auto& b : bullets) b.Draw();
    for (const auto& g : grenades) g.Draw();

    // Warrior body colour: orange for team 1, blue for team 2
    double cr, cg, cb;
    if (team == TEAM1) { cr=1.00; cg=0.45; cb=0.00; }
    else               { cr=0.05; cg=0.30; cb=0.95; }

    // Larger body (sz=1.2) so warriors are the biggest and most prominent
    drawBase(cr, cg, cb, 'W', 1.2);

    // ---- Ammo bar: drawn below the body ----
    double sz   = 1.2;
    double barW = sz * 2.0;
    double barH = 0.30;
    double barX0 = x - sz;
    double barY0 = y - sz - barH - 0.10;
    double frac  = (double)ammo / (double)maxAmmo;
    if (frac < 0) frac = 0;

    // Empty background (dark grey)
    glColor3d(0.20, 0.20, 0.20);
    glBegin(GL_QUADS);
    glVertex2d(barX0,      barY0);
    glVertex2d(barX0+barW, barY0);
    glVertex2d(barX0+barW, barY0+barH);
    glVertex2d(barX0,      barY0+barH);
    glEnd();

    // Filled portion (purple = ammo)
    glColor3d(0.65, 0.10, 0.85);
    glBegin(GL_QUADS);
    glVertex2d(barX0,           barY0);
    glVertex2d(barX0+barW*frac, barY0);
    glVertex2d(barX0+barW*frac, barY0+barH);
    glVertex2d(barX0,           barY0+barH);
    glEnd();

    // Border
    glColor3d(0.0, 0.0, 0.0);
    glBegin(GL_LINE_LOOP);
    glVertex2d(barX0,      barY0);
    glVertex2d(barX0+barW, barY0);
    glVertex2d(barX0+barW, barY0+barH);
    glVertex2d(barX0,      barY0+barH);
    glEnd();

    // ---- Current FSM state name (displayed above the HP bar) ----
    if (pCurrentState)
    {
        const char* stateName = pCurrentState->getName();
        // Colour-code the state text for quick reading
        if (stateName[0] == 'A') glColor3d(1.0, 0.2, 0.0);      // ATTACK  = red-orange
        else if (stateName[0] == 'F') glColor3d(0.0, 1.0, 0.3); // FLEE    = bright green
        else if (stateName[0] == 'G') glColor3d(0.9, 0.9, 0.0); // Get*    = yellow
        else glColor3d(0.85, 0.85, 0.85);                        // Search  = light grey

        glRasterPos2d(x - sz, y + sz + 0.55);
        for (const char* p = stateName; *p; p++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *p);
    }
}
