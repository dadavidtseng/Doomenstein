//----------------------------------------------------------------------------------------------------
// Weapon.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Weapon.hpp"

#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Game/Actor.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/PlayerController.hpp"
#include "Game/WeaponDefinition.hpp"

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
        if (m_owner == nullptr) return;
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
                // DebugAddWorldPoint(result.m_impactPosition, 0.06f, 10.f);
                DebugAddWorldCylinder(fireEyePosition - Vec3::Z_BASIS * 0.05f, result.m_impactPosition, 0.01f, 10.f, false, Rgba8::BLUE, Rgba8::BLUE, DebugRenderMode::X_RAY);
                DebugAddWorldCylinder(fireEyePosition - Vec3::Z_BASIS * 0.05f, result.m_impactPosition, 0.01f, 10.f, false, Rgba8::BLUE, Rgba8::BLUE, DebugRenderMode::USE_DEPTH);
            }
            else
            {
                DebugAddWorldCylinder(fireEyePosition - Vec3::Z_BASIS * 0.05f, result.m_rayStartPosition + result.m_rayForwardNormal * rayRange, 0.01f, 10.f, false, Rgba8::BLUE, Rgba8::BLUE, DebugRenderMode::USE_DEPTH);
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
            spawnInfo.m_name        = m_definition->m_projectileActor;
            spawnInfo.m_faction     = m_owner->m_definition->m_faction;
            spawnInfo.m_position    = startPos;
            spawnInfo.m_orientation = randomDirection;
            spawnInfo.m_velocity    = projectileSpeed;
            Actor* projectile       = m_owner->m_map->SpawnActor(spawnInfo);
            projectile->m_owner     = m_owner;
            projectileCount--;
        }

        while (meleeCount > 0)
        {
            meleeCount--;
            if (!m_owner || !m_owner->m_map) continue;

            // Forward vector from the owner's orientation
            Vec3 fwd, left, up;
            m_owner->m_orientation.GetAsVectors_IFwd_JLeft_KUp(fwd, left, up);

            Vec2  ownerPos2D = Vec2(m_owner->m_position.x, m_owner->m_position.y);
            Vec2  forward2D  = Vec2(fwd.x, fwd.y);
            float halfArc    = m_definition->m_meleeArc * 0.5f;

            Actor* bestTarget   = nullptr;
            float  bestDistSq   = FLT_MAX;
            float  meleeRangeSq = m_definition->m_meleeRange * m_definition->m_meleeRange;

            for (Actor* testActor : m_owner->m_map->m_actors)
            {
                if (!testActor || testActor == m_owner) continue;
                if (testActor->m_isDead) continue;
                if (testActor->m_definition->m_faction == m_owner->m_definition->m_faction) continue;
                if (testActor->m_definition->m_faction == "NEUTRAL" || m_owner->m_definition->m_faction == "NEUTRAL") continue;

                Vec2  testPos2D   = Vec2(testActor->m_position.x, testActor->m_position.y);
                float distSquared = GetDistanceSquared2D(ownerPos2D, testPos2D);
                if (distSquared > meleeRangeSq) continue;
                Vec2  toTarget2D = (testPos2D - ownerPos2D).GetNormalized();
                float angle      = GetAngleDegreesBetweenVectors2D(forward2D, toTarget2D);
                if (angle > halfArc) continue;

                if (distSquared < bestDistSq)
                {
                    bestDistSq = distSquared;
                    bestTarget = testActor;
                }
            }
            if (bestTarget)
            {
                float damage = g_theRNG->RollRandomFloatInRange(m_definition->m_meleeDamage.m_min, m_definition->m_meleeDamage.m_max);
                bestTarget->Damage((int)damage, m_owner->m_handle);
                bestTarget->AddImpulse(m_definition->m_meleeImpulse * fwd);
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------
// This, and other utility methods, will be helpful for randomizing weapons with a cone.
EulerAngles Weapon::GetRandomDirectionInCone(EulerAngles weaponOrientation, float degreeOfVariation)
{
    float const       randomYaw       = g_theRNG->RollRandomFloatInRange(-degreeOfVariation, degreeOfVariation);
    float const       randomPitch     = g_theRNG->RollRandomFloatInRange(-degreeOfVariation, degreeOfVariation);
    float const       randomRow       = g_theRNG->RollRandomFloatInRange(-degreeOfVariation, degreeOfVariation);
    EulerAngles const randomDirection = EulerAngles(weaponOrientation.m_yawDegrees + randomYaw, weaponOrientation.m_pitchDegrees + randomPitch, weaponOrientation.m_rollDegrees + randomRow);
    return randomDirection;
}
