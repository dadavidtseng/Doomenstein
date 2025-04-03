//----------------------------------------------------------------------------------------------------
// Weapon.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once

class Timer;
//-Forward-Declaration--------------------------------------------------------------------------------
class Actor;
struct WeaponDefinition;

//----------------------------------------------------------------------------------------------------
class Weapon
{
public:
    explicit Weapon(Actor* owner, WeaponDefinition const* weaponDef);

    void Fire();
    void GetRandomDirectionInCone();

    Actor*                  m_owner      = nullptr;
    WeaponDefinition const* m_definition = nullptr;     // Reference to our weapon definition.
    Timer *            m_timer      = nullptr;
};
