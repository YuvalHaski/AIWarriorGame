#pragma once
#include "NPC.h"

class SupplyNPC : public NPC
{
public:
    int ammoStock;
    int depotRow, depotCol;

    SupplyNPC(double startX, double startY, int team, int depotR, int depotC);

    void DoSomeWork() override;
    void show() override;

private:
    bool goingToDepot;
    int  supplyRange;
    int  supplyCooldown;

    static const int SUPPLY_MAX_STOCK = 60;
};
