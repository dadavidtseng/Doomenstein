//----------------------------------------------------------------------------------------------------
// AI.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/AIController.hpp"

#include "WeaponDefinition.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Actor.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/Map.hpp"
#include "Game/Weapon.hpp"

//----------------------------------------------------------------------------------------------------
AIController::AIController(Map* map)
    : Controller(map)
{
}

void AIController::Update(float const deltaSeconds)
{
    Actor* possessedActor = m_map->GetActorByHandle(m_actorHandle);

    if (possessedActor == nullptr) return;
    if (possessedActor->m_isDead) return;

    Actor const* target = m_map->GetClosestVisibleEnemy(possessedActor);

    if (target != nullptr &&
        // m_targetActorHandle.IsValid() &&
        m_targetActorHandle != target->m_handle &&
        !target->m_isDead)
    {
        m_targetActorHandle = target->m_handle;
    }

    Actor const* targetActor = m_map->GetActorByHandle(m_targetActorHandle);

    if (!targetActor ||
        targetActor->m_isDead)
    {
        m_targetActorHandle = ActorHandle::INVALID;
        return;
    }

    float const turnSpeedPerSecond          = possessedActor->m_definition->m_turnSpeed;
    float const maxTurnDegreesThisFrame     = turnSpeedPerSecond * deltaSeconds;
    Vec3        possessedActorToTargetActor = targetActor->m_position - possessedActor->m_position;
    possessedActorToTargetActor.z           = 0.f;

    float const targetActorYaw    = Atan2Degrees(possessedActorToTargetActor.y, possessedActorToTargetActor.x);
    float const possessedActorYaw = possessedActor->m_orientation.m_yawDegrees;
    float const newYaw            = GetTurnedTowardDegrees(possessedActorYaw, targetActorYaw, maxTurnDegreesThisFrame);

    EulerAngles const newDirection = EulerAngles(newYaw, possessedActor->m_orientation.m_pitchDegrees, possessedActor->m_orientation.m_rollDegrees);
    possessedActor->TurnInDirection(newDirection);

    float const distanceToTarget = possessedActorToTargetActor.GetLength();
    float const combinedRadius   = possessedActor->m_radius + targetActor->m_radius;

    if (distanceToTarget > combinedRadius + 0.1f)
    {
        float const moveSpeed = possessedActor->m_definition->m_runSpeed;
        Vec3        forward, left, up;
        possessedActor->m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
        possessedActor->MoveInDirection(forward, moveSpeed);
        possessedActor->PlayAnimationByName("Walk");
    }

    if (possessedActor->m_currentWeapon &&
        possessedActor->m_currentWeapon->m_definition->m_meleeCount > 0)
    {
        if (distanceToTarget < possessedActor->m_currentWeapon->m_definition->m_meleeRange + targetActor->m_radius)
        {
            possessedActor->m_currentWeapon->Fire();
            possessedActor->PlayAnimationByName("Attack", true);
        }
    }
}

//----------------------------------------------------------------------------------------------------
// Notification that the AI actor was damaged so this AI can target them.
void AIController::DamagedBy(ActorHandle const& attacker)
{
    m_targetActorHandle = attacker;
}
