#pragma once
#include "State.h"
#include "NPC.h"

const int WarehouseX = 22;
const int WarehouseY = 12;

class GoToWarehouse :
    public State
{
public:
	void OnEnter(NPC* pn);
	void Transition(NPC* pn);
	void OnExit(NPC* pn);


};

