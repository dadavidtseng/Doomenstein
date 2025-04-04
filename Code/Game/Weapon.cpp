//----------------------------------------------------------------------------------------------------
// Weapon.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Weapon.hpp"

#include "Actor.hpp"
#include "ActorDefinition.hpp"
#include "Game.hpp"
#include "GameCommon.hpp"
#include "Map.hpp"
#include "MapDefinition.hpp"
#include "PlayerController.hpp"
#include "WeaponDefinition.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
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

        while (rayCount > 0)
        {
            float             rayRange = m_definition->m_rayRange;
            Vec3              forward, left, up;
            Vec3 const        firePosition    = m_owner->m_position;
            Vec3 const        fireEyePosition = firePosition + Vec3(0.f, 0.f, m_owner->m_definition->m_eyeHeight);
            EulerAngles const fireOrientation = m_owner->m_orientation;
            fireOrientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
            ActorHandle           impactedActorHandle;
            RaycastResult3D const result = m_owner->m_map->RaycastAll(m_owner, impactedActorHandle, fireEyePosition, forward, 10.f);


            if (result.m_didImpact)
            {
                DebugAddWorldPoint(result.m_impactPosition, 0.06f, 10.f);
                DebugAddWorldWireCylinder(fireEyePosition, result.m_impactPosition, 0.01f, 10.f, Rgba8::BLUE, Rgba8::BLUE);
            }
            else
            {
                DebugAddWorldWireCylinder(fireEyePosition, result.m_rayStartPosition + result.m_rayForwardNormal * rayRange, 0.01f, 100.f, Rgba8::YELLOW, Rgba8::YELLOW, DebugRenderMode::X_RAY);
            }
            Actor* impactedActor = m_owner->m_map->GetActorByHandle(impactedActorHandle);
            if (impactedActor != nullptr && impactedActor != m_owner)
            {
                impactedActor->Damage((int)m_definition->m_rayDamage.m_min, m_owner->m_handle);
                impactedActor->AddImpulse(m_definition->m_rayImpulse * forward);
            }
            rayCount--;
        }

        while (projectileCount > 0)
        {
            Vec3 startPos = m_owner->m_position + Vec3(0.f, 0.f, m_owner->m_definition->m_eyeHeight);
            Vec3 forward, left, up;

            EulerAngles randomDirection = GetRandomDirectionInCone(m_owner->m_orientation, m_definition->m_projectileCone);
            randomDirection.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
            Vec3      projectileSpeed = forward * m_definition->m_projectileSpeed;
            SpawnInfo spawnInfo;
            spawnInfo.m_name = m_definition->m_projectileActor;
            spawnInfo.m_faction = m_owner->m_definition->m_faction;
            spawnInfo.m_position = startPos;
            spawnInfo.m_orientation = randomDirection;
            spawnInfo.m_velocity = projectileSpeed;
            Actor*    projectile = m_owner->m_map->SpawnActor(spawnInfo);
            projectile->m_owner  = m_owner;
            projectileCount--;
        }

        while (meleeCount > 0)
        {
            Vec2 point = Vec2(m_owner->m_position.x, m_owner->m_position.y);
            Vec3 forward, left, right;
            m_owner->m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, right);
            Vec2 forwardXY = Vec2(forward.x, forward.y);
            Vec2 sectorTip = point + forwardXY;
            for (Actor* actor : m_owner->m_map->m_actors)
            {
                Vec2 actorPosition = Vec2(actor->m_position.x, actor->m_position.y);
                if (IsPointInsideDirectedSector2D(actorPosition, sectorTip, forwardXY, 120.f, m_owner->m_definition->m_sightRadius))
                {
                    DebuggerPrintf("%s\n", actor->m_definition->m_name.c_str());
                }
            }

            meleeCount--;
        }
    }
}

//----------------------------------------------------------------------------------------------------
// This, and other utility methods, will be helpful for randomizing weapons with a cone.
EulerAngles Weapon::GetRandomDirectionInCone(EulerAngles weaponOrientation, float degreeOfVariation)
{
    float       variationYaw   = g_theRNG->RollRandomFloatInRange(-degreeOfVariation, degreeOfVariation);
    float       variationPitch = g_theRNG->RollRandomFloatInRange(-degreeOfVariation, degreeOfVariation);
    float       variationRow   = g_theRNG->RollRandomFloatInRange(-degreeOfVariation, degreeOfVariation);
    EulerAngles newDirection   = EulerAngles(weaponOrientation.m_yawDegrees + variationYaw, weaponOrientation.m_pitchDegrees + variationPitch, weaponOrientation.m_rollDegrees + variationRow);
    return newDirection;
}
