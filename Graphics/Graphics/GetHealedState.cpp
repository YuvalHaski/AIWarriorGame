#include "GetHealedState.h"
#include "WarriorNPC.h"
#include "MedicNPC.h"
#include "SearchEnemyState.h"
#include "FleeState.h"
#include "GetAmmoState.h"
#include "DungeonMap.h"
#include <cmath>

void GetHealedState::OnEnter(WarriorNPC* pn)
{
    NPC* medic = pn->FindFriendlyMedic();
    if (medic)
        pn->PlanPathTo(medic->getGridRow(), medic->getGridCol());
}

//void GetHealedState::Transition(WarriorNPC* pn)
//{
//    // Priority 1: Flee (even when wanting healing, critical situations override)
//    if (pn->getHp() <= pn->fleeHpThreshold || pn->getAmmo() <= pn->fleeAmmoThreshold)
//    {
//        pn->ChangeState(new FleeState());
//        return;
//    }
//
//    NPC* medic = pn->FindFriendlyMedic();
//    if (!medic)
//    {
//        pn->ChangeState(new SearchEnemyState());
//        return;
//    }
//
//    // Re-plan if medic moved
//    double dx = medic->getX() - pn->getPathEndX();
//    double dy = medic->getY() - pn->getPathEndY();
//    if (std::sqrt(dx*dx + dy*dy) > 3.0)
//        pn->PlanPathTo(medic->getGridRow(), medic->getGridCol());
//
//    bool done = pn->FollowPlannedPath();
//
//    if (done || (std::abs(pn->getX() - medic->getX()) < 2.0 &&
//                 std::abs(pn->getY() - medic->getY()) < 2.0))
//    {
//        // MedicNPC handles the healing; once HP OK move on
//        if (pn->getHp() > pn->getHealThreshold)
//        {
//            if (pn->getAmmo() <= pn->getAmmoThreshold)
//                pn->ChangeState(new GetAmmoState());
//            else
//                pn->ChangeState(new SearchEnemyState());
//        }
//    }
//}

void GetHealedState::Transition(WarriorNPC* pn)
{
    NPC* nearestMedic = nullptr;
    double minDist = 1e9;

    // Requirement: Turn to the Medic for help
    for (NPC* other : allNPCs)
    {
        if (other->getType() == NPC_MEDIC &&
            other->getTeam() == pn->getTeam() &&
            other->isAlive())
        {
            double d = std::sqrt(std::pow(pn->getX() - other->getX(), 2) +
                std::pow(pn->getY() - other->getY(), 2));
            if (d < minDist) {
                minDist = d;
                nearestMedic = other;
            }
        }
    }

    if (nearestMedic)
    {
        // Dynamic target: The Medic's current position
        pn->PlanPathTo((int)nearestMedic->getY(), (int)nearestMedic->getX());
    }
    else
    {
        // Fallback: The static medical depot
        int tr = (pn->getTeam() == TEAM1) ? medDepot1Row : medDepot2Row;
        int tc = (pn->getTeam() == TEAM1) ? medDepot1Col : medDepot2Col;
        pn->PlanPathTo(tr, tc);
    }

    pn->FollowPlannedPath();

    // If reached destination and healed, transition back
    if (pn->getHp() > pn->getHealThreshold)
    {
        pn->ChangeState(new SearchEnemyState());
    }
}

void GetHealedState::OnExit(WarriorNPC* pn)
{
    (void)pn;
}
