//----------------------------------------------------------------------------------------------------
// Weapon.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include "Engine/Math/EulerAngles.hpp"

//-Forward-Declaration--------------------------------------------------------------------------------
class Actor;
class Timer;
struct WeaponDefinition;

//----------------------------------------------------------------------------------------------------
class Weapon
{
public:
    explicit Weapon(Actor* owner, WeaponDefinition const* weaponDef);

    void Fire();
    EulerAngles GetRandomDirectionInCone(EulerAngles weaponOrientation, float degreeOfVariation);

    Actor*                  m_owner      = nullptr;
    WeaponDefinition const* m_definition = nullptr;     // Reference to our weapon definition.
    Timer *            m_timer      = nullptr;
};
