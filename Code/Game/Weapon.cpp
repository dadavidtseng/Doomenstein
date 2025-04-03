//----------------------------------------------------------------------------------------------------
// Weapon.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Weapon.hpp"

#include "WeaponDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

Weapon::Weapon(Actor*                  owner,
               WeaponDefinition const* weaponDef)
    : m_owner(owner),
      m_definition(weaponDef)
{

}

//----------------------------------------------------------------------------------------------------
// Checks if the weapon is ready to fire.
// If so, fires each of the ray casts, projectiles, and melee attacks defined in the definition.
// Needs to pass along its owning actor to be ignored in all raycast and collision checks.
void Weapon::Fire()
{
    DebuggerPrintf("FIRE by: %s\n", m_definition->m_name.c_str());
}

//----------------------------------------------------------------------------------------------------
// This, and other utility methods, will be helpful for randomizing weapons with a cone.
void Weapon::GetRandomDirectionInCone()
{
}
