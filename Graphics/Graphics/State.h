#pragma once

class WarriorNPC;

class State
{
public:
    virtual void OnEnter(WarriorNPC* pn) = 0;
    virtual void Transition(WarriorNPC* pn) = 0;
    virtual void OnExit(WarriorNPC* pn) = 0;
    virtual const char* getName() const = 0;
    virtual ~State() {}
};
