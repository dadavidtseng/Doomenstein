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
    explicit Weapon(Actor* owner, WeaponDefinition const* weaponDef);

    void Fire();
    void getRandomDirectionInCone();

    Actor*            m_owner            = nullptr;
    WeaponDefinition* m_weaponDefinition = nullptr;
};
