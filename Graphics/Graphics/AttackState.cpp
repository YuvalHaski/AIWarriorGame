#include "AttackState.h"
#include "WarriorNPC.h"
#include "SearchEnemyState.h"
#include "FleeState.h"
#include "GetAmmoState.h"
#include "GetHealedState.h"
#include "SecurityMap.h"
#include <cstdlib>

void AttackState::OnEnter(WarriorNPC* pn)
{
    // Raise security in current room when combat starts
    int rid = pn->getCurrentRoom();
    if (rid >= 0)
        SecurityMapRaiseDanger(rid, 0.3);
}

void AttackState::Transition(WarriorNPC* pn)
{
    // Priority 1: Flee
    if (pn->getHp() <= pn->fleeHpThreshold || pn->getAmmo() <= pn->fleeAmmoThreshold)
    {
        pn->ChangeState(new FleeState());
        return;
    }

    // Priority 2: Get healed
    if (pn->getHp() <= pn->getHealThreshold)
    {
        pn->ChangeState(new GetHealedState());
        return;
    }

    // Priority 3: Get ammo
    if (pn->getAmmo() <= pn->getAmmoThreshold)
    {
        pn->ChangeState(new GetAmmoState());
        return;
    }

    // Find enemy in same room
    NPC* enemy = pn->FindNearestEnemy();
    if (!enemy || enemy->getCurrentRoom() != pn->getCurrentRoom() ||
        pn->getCurrentRoom() < 0)
    {
        // Enemy left the room or no enemy; go search
        pn->ChangeState(new SearchEnemyState());
        return;
    }

    // Attack: cooldown controlled
    if (pn->attackCooldown <= 0)
    {
        if (pn->getAmmo() > 0)
        {
            // Aggressive warriors prefer grenade; cautious prefer bullet
            double grenadeChance = 0.3 * pn->aggression;
            if ((rand() % 100) < (int)(grenadeChance * 100) && pn->getAmmo() >= 3)
                pn->FireGrenade(enemy);
            else
                pn->FireBullet(enemy);
        }
        pn->attackCooldown = COMBAT_TICK_INTERVAL;
    }
    else
    {
        pn->attackCooldown--;
    }
}

void AttackState::OnExit(WarriorNPC* pn)
{
    (void)pn;
}
