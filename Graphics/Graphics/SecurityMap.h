#pragma once
#include "Definitions.h"

// Raises danger in every cell belonging to the given room
void SecurityMapRaiseDanger(int roomID, double amount);

// Decays all security values toward zero each frame
void SecurityMapDecay();

