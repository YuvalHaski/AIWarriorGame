#pragma once
#include "State.h"

class FleeState : public State
{
public:
    void OnEnter(WarriorNPC* pn) override;
    void Transition(WarriorNPC* pn) override;
    void OnExit(WarriorNPC* pn) override;
    const char* getName() const override { return "FLEEING!"; }

private:
    int safeRoomIdx;
};
