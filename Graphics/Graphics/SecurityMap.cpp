#include "SecurityMap.h"
#include "DungeonMap.h" 
#include "NPC.h"
#include <algorithm>

void SecurityMapRaiseDanger(int roomID, double amount)
{
    for (int r = 0; r < MSZ; r++)
        for (int c = 0; c < MSZ; c++)
            if (roomId[r][c] == roomID)
            {
                securityMap[r][c] += amount;
                if (securityMap[r][c] > 1.0)
                    securityMap[r][c] = 1.0;
            }
}

void SecurityMapDecay()
{
    for (int r = 0; r < MSZ; r++)
        for (int c = 0; c < MSZ; c++)
        {
            securityMap[r][c] -= SECURITY_DECAY;
            if (securityMap[r][c] < 0.0)
                securityMap[r][c] = 0.0;
        }
}


// Helper to check if a room contains NPCs from opposing teams
bool IsRoomInCombat(int rId) {
    if (rId < 0) return false;
    bool team1Present = false;
    bool team2Present = false;
    for (NPC* n : allNPCs) {
        if (n->isAlive() && n->getCurrentRoom() == rId) {
            if (n->getTeam() == TEAM1) team1Present = true;
            if (n->getTeam() == TEAM2) team2Present = true;
        }
    }
    return team1Present && team2Present;
}

void SecurityMapUpdate()
{
    // Requirement: Only update security where combat is happening
    for (const auto& room : rooms)
    {
        if (IsRoomInCombat(room.id))
        {
            for (int r = room.row; r < room.row + room.height; r++)
            {
                for (int c = room.col; c < room.col + room.width; c++)
                {
                    // Decay the risk over time in combat rooms
                    securityMap[r][c] *= (1.0 - SECURITY_DECAY);
                }
            }
        }
    }
}
