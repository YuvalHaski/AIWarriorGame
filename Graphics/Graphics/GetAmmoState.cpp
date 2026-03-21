#include "GetAmmoState.h"
#include "WarriorNPC.h"
#include "SearchEnemyState.h"
#include "SupplyNPC.h"
#include "FleeState.h"
#include "GetHealedState.h"
#include "DungeonMap.h"
#include <cmath>

void GetAmmoState::OnEnter(WarriorNPC* pn)
{
    // Go to nearest supply soldier
    NPC* supply = pn->FindFriendlySupply();
    if (supply)
        pn->PlanPathTo(supply->getGridRow(), supply->getGridCol());
    else
    {
        // Fallback: go directly to ammo depot
        int tr = (pn->getTeam() == TEAM1) ? ammoDepot1Row : ammoDepot2Row;
        int tc = (pn->getTeam() == TEAM1) ? ammoDepot1Col : ammoDepot2Col;
        pn->PlanPathTo(tr, tc);
    }
}

void GetAmmoState::Transition(WarriorNPC* pn)
{
    // Priority 1: Flee
    if (pn->getHp() <= pn->fleeHpThreshold || pn->getAmmo() <= pn->fleeAmmoThreshold)
    {
        pn->ChangeState(new FleeState());
        return;
    }

    // Priority 2: Heal
    if (pn->getHp() <= pn->getHealThreshold)
    {
        pn->ChangeState(new GetHealedState());
        return;
    }

    // Move toward supply soldier
    NPC* supply = pn->FindFriendlySupply();
    if (!supply)
    {
        // No supply soldier alive, go search
        pn->ChangeState(new SearchEnemyState());
        return;
    }

    // Re-plan only if supply moved significantly from path destination
    double dx = supply->getX() - pn->getPathEndX();
    double dy = supply->getY() - pn->getPathEndY();
    if (std::sqrt(dx*dx + dy*dy) > 3.0)
        pn->PlanPathTo(supply->getGridRow(), supply->getGridCol());

    bool done = pn->FollowPlannedPath();

    // Close enough? Supply soldier will hand over ammo automatically (handled in SupplyNPC)
    if (done || (std::abs(pn->getX() - supply->getX()) < 2.0 &&
                 std::abs(pn->getY() - supply->getY()) < 2.0))
    {
        // Ammo refill is given by the SupplyNPC; once ammo OK, move on
        if (pn->getAmmo() > pn->getAmmoThreshold)
        {
            pn->ChangeState(new SearchEnemyState());
        }
    }
}

void GetAmmoState::OnExit(WarriorNPC* pn)
{
    (void)pn;
}
