#include "FleeState.h"
#include "WarriorNPC.h"
#include "SearchEnemyState.h"
#include "GetHealedState.h"
#include "GetAmmoState.h"
#include "DungeonMap.h"
#include <cstdlib>
#include <cfloat>
#include <cmath>

void FleeState::OnEnter(WarriorNPC* pn)
{
    // Find the room with the lowest security (safest room)
    safeRoomIdx = 0;
    double lowestRisk = DBL_MAX;
    for (int i = 0; i < (int)rooms.size(); i++)
    {
        int cr = rooms[i].centerRow();
        int cc = rooms[i].centerCol();
        if (cr >= 0 && cr < MSZ && cc >= 0 && cc < MSZ)
        {
            double risk = securityMap[cr][cc];
            if (risk < lowestRisk)
            {
                lowestRisk = risk;
                safeRoomIdx = i;
            }
        }
    }
    // Don't flee to current room
    if (safeRoomIdx == pn->getCurrentRoom() && numRooms > 1)
        safeRoomIdx = (safeRoomIdx + 1) % numRooms;

    Room& rm = rooms[safeRoomIdx];
    pn->PlanPathTo(rm.centerRow(), rm.centerCol());
}

void FleeState::Transition(WarriorNPC* pn)
{
    // Keep moving along flee path
    pn->FollowPlannedPath();

    // Once we're no longer critically low, re-evaluate
    bool criticalHp    = pn->getHp()   <= pn->fleeHpThreshold;
    bool criticalAmmo  = pn->getAmmo() <= pn->fleeAmmoThreshold;

    if (!criticalHp && !criticalAmmo)
    {
        // Transition based on current state
        if (pn->getHp() <= pn->getHealThreshold)
        {
            pn->ChangeState(new GetHealedState());
            return;
        }
        if (pn->getAmmo() <= pn->getAmmoThreshold)
        {
            pn->ChangeState(new GetAmmoState());
            return;
        }
        pn->ChangeState(new SearchEnemyState());
    }
}

void FleeState::OnExit(WarriorNPC* pn)
{
    (void)pn;
}
