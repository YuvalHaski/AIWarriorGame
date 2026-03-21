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
    // Only decay cells that actually have danger (efficiency + per task Note 2)
    for (int r = 0; r < MSZ; r++)
        for (int c = 0; c < MSZ; c++)
        {
            if (securityMap[r][c] > 0.0)
            {
                securityMap[r][c] -= SECURITY_DECAY;
                if (securityMap[r][c] < 0.0)
                    securityMap[r][c] = 0.0;
            }
        }
}
