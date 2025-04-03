//----------------------------------------------------------------------------------------------------
// Weapon.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Weapon.hpp"

#include "Actor.hpp"
#include "Game.hpp"
#include "GameCommon.hpp"
#include "Map.hpp"
#include "PlayerController.hpp"
#include "WeaponDefinition.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"

Weapon::Weapon(Actor*                  owner,
               WeaponDefinition const* weaponDef)
    : m_owner(owner),
      m_definition(weaponDef)
{
    m_timer              = new Timer(m_definition->m_refireTime, g_theGame->m_gameClock);
    m_timer->m_startTime = g_theGame->m_gameClock->GetTotalSeconds();
}

//----------------------------------------------------------------------------------------------------
// Checks if the weapon is ready to fire.
// If so, fires each of the ray casts, projectiles, and melee attacks defined in the definition.
// Needs to pass along its owning actor to be ignored in all raycast and collision checks.
void Weapon::Fire()
{
    int rayCount        = m_definition->m_rayCount;
    int projectileCount = m_definition->m_projectileCount;
    int meleeCount      = m_definition->m_meleeCount;


    if (m_timer->HasPeriodElapsed())
    {
        m_timer->DecrementPeriodIfElapsed();
        DebuggerPrintf("%f\n", m_timer->m_startTime);

        while (rayCount > 0)
        {
            Vec3                    forward, left, up;
            PlayerController const* playerController = dynamic_cast<PlayerController*>(m_owner->m_controller);
            Vec3 const              firePosition     = playerController->m_position;
            EulerAngles const       fireOrientation  = playerController->m_orientation;
            fireOrientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
            ActorHandle           impactedActorHandle;
            RaycastResult3D const result        = m_owner->m_map->RaycastAll(m_owner, impactedActorHandle, firePosition, forward, 10.f);
            Actor*                impactedActor = m_owner->m_map->GetActorByHandle(impactedActorHandle);

            if (result.m_didImpact)
            {
                DebugAddWorldPoint(result.m_impactPosition, 0.06f, 10.f);

                if (impactedActor != nullptr && impactedActor != m_owner)
                {
                    impactedActor->Damage((int)m_definition->m_rayDamage.m_min, m_owner->m_handle);
                }
            }
            rayCount--;
        }
    }
}

//----------------------------------------------------------------------------------------------------
// This, and other utility methods, will be helpful for randomizing weapons with a cone.
void Weapon::GetRandomDirectionInCone()
{
}
