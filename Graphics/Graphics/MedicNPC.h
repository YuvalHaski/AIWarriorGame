#pragma once
#include "NPC.h"

class MedicNPC : public NPC
{
public:
    int healCooldown;
    int medicineStock;     // how many heal charges the medic has
    int depotRow, depotCol; // which med depot this medic uses

    MedicNPC(double startX, double startY, int team, int depotR, int depotC);

    void DoSomeWork() override;
    void show() override;

private:
    bool goingToDepot;
    int  healRange;
};
