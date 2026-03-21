#pragma once
#include "NPC.h"
#include "State.h"
#include "Bullet.h"
#include "Grenade.h"
#include <vector>

class WarriorNPC : public NPC
{
public:
    // Personality traits
    double aggression;          // 0.5 to 1.0
    int    fleeHpThreshold;     // flee below this HP
    int    fleeAmmoThreshold;   // flee below this ammo
    int    getAmmoThreshold;    // go get ammo below this
    int    getHealThreshold;    // go get healed below this HP

    int    ammo;
    int    maxAmmo;

    // FSM
    State* pCurrentState;

    // Combat cooldown
    int    attackCooldown;

    // Fired projectiles (shared with the world)
    std::vector<Bullet>  bullets;
    std::vector<Grenade> grenades;

    WarriorNPC(double startX, double startY, int team);
    ~WarriorNPC();

    void DoSomeWork() override;
    void show() override;

    // Fire at a target NPC
    void FireBullet(NPC* target);
    void FireGrenade(NPC* target);

    int  getAmmo()    const { return ammo; }
    void addAmmo(int a)     { ammo = std::min(ammo + a, maxAmmo); }

    // Returns pointer to the nearest living enemy, or nullptr
    NPC* FindNearestEnemy() const;

    // Returns pointer to the nearest friendly Medic, or nullptr
    NPC* FindFriendlyMedic() const;

    // Returns pointer to the nearest friendly Supply, or nullptr
    NPC* FindFriendlySupply() const;

    // Change FSM state
    void ChangeState(State* newState);

    // Collect projectile refs for the game loop to draw/update
    std::vector<Bullet>&  getBullets()  { return bullets;  }
    std::vector<Grenade>& getGrenades() { return grenades; }
};
