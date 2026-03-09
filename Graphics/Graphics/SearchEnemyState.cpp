#include "SearchEnemyState.h"
#include "WarriorNPC.h"
#include "AttackState.h"
#include "FleeState.h"
#include "GetAmmoState.h"
#include "GetHealedState.h"
#include "DungeonMap.h"
#include <cstdlib>

void SearchEnemyState::OnEnter(WarriorNPC* pn)
{
    patrolRoomIndex = (numRooms > 0) ? rand() % numRooms : 0;
    patrolTimer = 0;
    // Plan path to a random room
    if (numRooms > 0)
    {
        Room& rm = rooms[patrolRoomIndex];
        pn->PlanPathTo(rm.centerRow(), rm.centerCol());
    }
}

void SearchEnemyState::Transition(WarriorNPC* pn)
{
    // Priority 1: Flee if critically low
    if (pn->getHp() <= pn->fleeHpThreshold || pn->getAmmo() <= pn->fleeAmmoThreshold)
    {
        pn->ChangeState(new FleeState());
        return;
    }

    // Priority 2: Get healed if HP low
    if (pn->getHp() <= pn->getHealThreshold)
    {
        pn->ChangeState(new GetHealedState());
        return;
    }

    // Priority 3: Get ammo if low
    if (pn->getAmmo() <= pn->getAmmoThreshold)
    {
        pn->ChangeState(new GetAmmoState());
        return;
    }

    // Priority 4: Attack if enemy in same room
    NPC* enemy = pn->FindNearestEnemy();
    if (enemy && enemy->getCurrentRoom() == pn->getCurrentRoom() &&
        pn->getCurrentRoom() >= 0)
    {
        pn->ChangeState(new AttackState());
        return;
    }

    // Continue patrolling: if path complete, pick next room
    patrolTimer++;
    bool done = pn->FollowPlannedPath();
    if (done || patrolTimer > 300)
    {
        patrolTimer = 0;
        // Pick a different room
        int next = (numRooms > 1)
            ? (patrolRoomIndex + 1 + rand() % (numRooms - 1)) % numRooms
            : 0;
        patrolRoomIndex = next;
        if (numRooms > 0)
        {
            Room& rm = rooms[patrolRoomIndex];
            pn->PlanPathTo(rm.centerRow(), rm.centerCol());
        }
    }
}

void SearchEnemyState::OnExit(WarriorNPC* pn)
{
    (void)pn;
}
