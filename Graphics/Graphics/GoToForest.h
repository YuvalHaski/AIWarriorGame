#pragma once
#include "State.h"
#include "NPC.h"

const int ForestX = 85;
const int ForestY = 85;

class GoToForest :
    public State
{
public:
	 void OnEnter(NPC* pn) ;
	 void Transition(NPC* pn);
	 void OnExit(NPC* pn) ;

};

