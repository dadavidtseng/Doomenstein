//----------------------------------------------------------------------------------------------------
// Weapon.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once

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
    float                   m_refireTime = 0.f;
    int m_count = 0;
    WeaponDefinition const* m_definition = nullptr;     // Reference to our weapon definition.
};
