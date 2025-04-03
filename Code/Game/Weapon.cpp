//----------------------------------------------------------------------------------------------------
// Weapon.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Weapon.hpp"

#include "Actor.hpp"
#include "Map.hpp"
#include "PlayerController.hpp"
#include "WeaponDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"

Weapon::Weapon(Actor*                  owner,
               WeaponDefinition const* weaponDef)
    : m_owner(owner),
      m_definition(weaponDef)
{
    switch (m_definition->m_name)
    {
    case "Pistol":
        {
            m_refireTime = m_definition.
        }
    }
}

//----------------------------------------------------------------------------------------------------
// Checks if the weapon is ready to fire.
// If so, fires each of the ray casts, projectiles, and melee attacks defined in the definition.
// Needs to pass along its owning actor to be ignored in all raycast and collision checks.
void Weapon::Fire()
{
    Vec3                    forward, left, up;
    PlayerController const* playerController = dynamic_cast<PlayerController*>(m_owner->m_controller);
    Vec3 const              firePosition     = playerController->m_position;
    EulerAngles const       fireOrientation  = playerController->m_orientation;
    fireOrientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
    RaycastResult3D const result = m_owner->m_map->RaycastAll(firePosition, forward, 10.f);

    if (result.m_didImpact)
    {
        DebugAddWorldPoint(result.m_impactPosition, 0.06f, 10.f);
    }
}

//----------------------------------------------------------------------------------------------------
// This, and other utility methods, will be helpful for randomizing weapons with a cone.
void Weapon::GetRandomDirectionInCone()
{
}
