//----------------------------------------------------------------------------------------------------
// AI.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/AIController.hpp"

#include "Actor.hpp"
#include "ActorDefinition.hpp"
#include "Map.hpp"
#include "Weapon.hpp"
#include "WeaponDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"

//----------------------------------------------------------------------------------------------------
AIController::AIController(Map* map)
    : Controller(map)
{
}

AIController::~AIController()
{
}

void AIController::Update(float deltaSeconds)
{
    Actor* controlledActor = m_map->GetActorByHandle(m_actorHandle);
    if (controlledActor == nullptr) return;
    if (controlledActor->m_isDead) return;

    Actor* target = m_map->GetClosestVisibleEnemy(controlledActor);

    if (target  && m_targetActorHandle != target->m_handle && !target->m_isDead)
    {
        m_targetActorHandle = target->m_handle;
    }
    Actor* targetActor = m_map->GetActorByHandle(m_targetActorHandle);
    if (!targetActor || targetActor->m_isDead)
    {
        m_targetActorHandle = ActorHandle::INVALID;
        return;
    }

    float turnSpeedDegPerSec      = controlledActor->m_definition->m_turnSpeed;
    float maxTurnDegreesThisFrame = turnSpeedDegPerSec * deltaSeconds;
    Vec3  toTarget3D              = targetActor->m_position - controlledActor->m_position;
    toTarget3D.z                  = 0.f;

    float desiredYaw = Atan2Degrees(toTarget3D.y, toTarget3D.x);
    float currentYaw = controlledActor->m_orientation.m_yawDegrees;
    float newYaw     = GetTurnedTowardDegrees(currentYaw, desiredYaw, maxTurnDegreesThisFrame);

    // Vec3 newDirectionTurningTo = Vec3(newYaw, controlledActor->m_orientation.m_pitchDegrees, controlledActor->m_orientation.m_rollDegrees);
    EulerAngles newDirectionTurningTo = EulerAngles (newYaw, controlledActor->m_orientation.m_pitchDegrees, controlledActor->m_orientation.m_rollDegrees);
    controlledActor->TurnInDirection(newDirectionTurningTo);

    float distanceToTarget = toTarget3D.GetLength();
    float combinedRadius   = controlledActor->m_radius + targetActor->m_radius;
    if (distanceToTarget > combinedRadius + 0.1f)
    {
        float moveSpeed = controlledActor->m_definition->m_runSpeed;
        Vec3  forward, left, up;
        controlledActor->m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
        controlledActor->MoveInDirection(forward, moveSpeed);
    }
    /// Hanlde melee weapon based on melee weapon range
    if (controlledActor->m_currentWeapon && controlledActor->m_currentWeapon->m_definition->m_meleeCount > 0)
    {
        if (distanceToTarget < controlledActor->m_currentWeapon->m_definition->m_meleeRange + targetActor->m_radius)
            controlledActor->m_currentWeapon->Fire();
    }
}

//----------------------------------------------------------------------------------------------------
// Notification that the AI actor was damaged so this AI can target them.
void AIController::DamagedBy(ActorHandle const& attacker)
{
    m_targetActorHandle = attacker;
}
