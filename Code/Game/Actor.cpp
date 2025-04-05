//----------------------------------------------------------------------------------------------------
// Actor.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Actor.hpp"

#include "ActorDefinition.hpp"
#include "AIController.hpp"
#include "Game.hpp"
#include "GameCommon.hpp"
#include "Map.hpp"
#include "MapDefinition.hpp"
#include "PlayerController.hpp"
#include "Tile.hpp"
#include "Weapon.hpp"
#include "WeaponDefinition.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Renderer.hpp"

// //----------------------------------------------------------------------------------------------------
// Actor::Actor(Vec3 const&        position,
//              EulerAngles const& orientation,
//              float const        radius,
//              float const        height,
//              bool const         isMovable,
//              Rgba8 const&       color)
//     : m_position(position),
//       m_orientation(orientation),
//       m_radius(radius),
//       m_height(height),
//       m_isMovable(isMovable),
//       m_color(color)
// {
//
//
//     m_cylinder = Cylinder3(m_position, m_position + Vec3(0.f, 0.f, m_height), m_radius);
// }

Actor::Actor(SpawnInfo const& spawnInfo)
{
    m_definition = ActorDefinition::GetDefByName(spawnInfo.m_name);

    if (m_definition == nullptr)
    {
        ERROR_AND_DIE("Failed to find actor definition")
    }

    m_health = m_definition->m_health;
    m_height = m_definition->m_height;
    // m_eyeHeight   = m_definition->m_eyeHeight;
    // m_cameraFOV   = m_definition->m_cameraFOV;
    m_radius      = m_definition->m_radius;
    m_position    = spawnInfo.m_position;
    m_orientation = spawnInfo.m_orientation;
    m_velocity    = spawnInfo.m_velocity;

    for (String const& weapon : m_definition->m_inventory)
    {
        if (WeaponDefinition const* weaponDef = WeaponDefinition::GetDefByName(weapon))
        {
            m_weapons.push_back(new Weapon(this, weaponDef));
        }
    }

    if (!m_weapons.empty() && m_weapons[0] != nullptr)
    {
        m_currentWeapon = m_weapons[0];
    }

    // for (int i = 0; i < (int)m_definition->m_inventory.size(); ++i)
    // {
    //     m_weapons.push_back(new Weapon(this, WeaponDefinition::GetDefByName(m_definition->m_inventory[i])));
    // }
    //
    // if (!m_weapons.empty())
    // {
    //     m_currentWeapon = m_weapons[0];
    // }


    // m_currentWeapon = new Weapon(this, )

    if (spawnInfo.m_name == "Marine")
    {
        m_color = Rgba8::GREEN;
    }

    if (spawnInfo.m_name == "Demon")
    {
        m_color = Rgba8::RED;
    }

    if (spawnInfo.m_name == "PlasmaProjectile")
    {
        m_color = Rgba8::BLUE;
    }

    // m_aiController      = new AIController(m_map);
    // m_controller = m_aiController;
    // m_aiController->Possess(m_handle);
    m_collisionCylinder = Cylinder3(m_position, m_position + Vec3(0.f, 0.f, m_height), m_radius);
}

//----------------------------------------------------------------------------------------------------
void Actor::Update(float const deltaSeconds)
{
    if (m_isDead||m_health<-1)
    {
        m_dead += deltaSeconds;
    }
    // if (m_dead > 3.f) m_isGarbage = true;
    if (m_dead > m_definition->m_corpseLifetime) m_isGarbage = true;

    if (!m_isDead)
    {
        UpdatePhysics(deltaSeconds);

    }

    if (m_aiController != nullptr)
    {
        m_aiController->Update(deltaSeconds);
    }

    m_collisionCylinder.m_startPosition = m_position;
    m_collisionCylinder.m_endPosition   = m_position + Vec3(0.f, 0.f, m_height);
}

//----------------------------------------------------------------------------------------------------
void Actor::UpdatePosition()
{
    float                 deltaSeconds   = static_cast<float>(Clock::GetSystemClock().GetDeltaSeconds());
    XboxController const& controller     = g_theInput->GetController(0);
    Vec2 const            leftStickInput = controller.GetLeftStick().GetPosition();

    Vec3 forward;
    Vec3 left;
    Vec3 up;
    m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);

    m_velocity                = Vec3::ZERO;
    float constexpr moveSpeed = 1.f;
    m_velocity += Vec3(leftStickInput.y, -leftStickInput.x, 0.f) * moveSpeed;

    if (g_theInput->IsKeyDown(KEYCODE_W)) m_velocity += forward * moveSpeed;
    if (g_theInput->IsKeyDown(KEYCODE_S)) m_velocity -= forward * moveSpeed;
    if (g_theInput->IsKeyDown(KEYCODE_A)) m_velocity += left * moveSpeed;
    if (g_theInput->IsKeyDown(KEYCODE_D)) m_velocity -= left * moveSpeed;
    if (g_theInput->IsKeyDown(KEYCODE_Z) || controller.IsButtonDown(XBOX_BUTTON_LSHOULDER)) m_velocity -= Vec3(0.f, 0.f, 1.f) * moveSpeed;
    if (g_theInput->IsKeyDown(KEYCODE_C) || controller.IsButtonDown(XBOX_BUTTON_RSHOULDER)) m_velocity += Vec3(0.f, 0.f, 1.f) * moveSpeed;
    if (g_theInput->IsKeyDown(KEYCODE_SHIFT) || controller.IsButtonDown(XBOX_BUTTON_A)) deltaSeconds *= 15.f;


    if (g_theGame->GetPlayerController() != nullptr)
    {
        m_orientation.m_yawDegrees = g_theGame->GetPlayerController()->m_orientation.m_yawDegrees;
    }

    m_position += m_velocity * deltaSeconds;
}

//----------------------------------------------------------------------------------------------------
// If visible, we will need vertexes and any other information necessary for rendering.
void Actor::Render() const
{
    if (m_definition->m_name == "SpawnPoint") return;
    if (!m_isVisible) return;

    VertexList_PCU verts;
    float const    eyeHeight         = m_definition->m_eyeHeight;
    Vec3 const     forwardNormal     = m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D().GetNormalized();
    Vec3 const     coneStartPosition = m_collisionCylinder.m_startPosition + Vec3(0.f, 0.f, eyeHeight) + forwardNormal * m_radius;

    if (m_definition->m_name != "PlasmaProjectile")
    {
        AddVertsForCone3D(verts, coneStartPosition, coneStartPosition + forwardNormal * 0.1f, 0.1f, m_color);
        AddVertsForWireframeCone3D(verts, coneStartPosition, coneStartPosition + forwardNormal * 0.1f, 0.1f, 0.001f);
    }

    if (m_isDead)
    {
        AddVertsForCylinder3D(verts, m_collisionCylinder.m_startPosition, m_collisionCylinder.m_endPosition, m_collisionCylinder.m_radius, Rgba8::LIGHT_BLUE);
    }
    else
    {
        AddVertsForCylinder3D(verts, m_collisionCylinder.m_startPosition, m_collisionCylinder.m_endPosition, m_collisionCylinder.m_radius, m_color);
    }

    AddVertsForWireframeCylinder3D(verts, m_collisionCylinder.m_startPosition, m_collisionCylinder.m_endPosition, m_collisionCylinder.m_radius, 0.001f);

    g_theRenderer->SetModelConstants();
    g_theRenderer->SetBlendMode(eBlendMode::OPAQUE);
    g_theRenderer->SetRasterizerMode(eRasterizerMode::SOLID_CULL_BACK);
    g_theRenderer->SetSamplerMode(eSamplerMode::POINT_CLAMP);
    g_theRenderer->SetDepthMode(eDepthMode::READ_WRITE_LESS_EQUAL);
    g_theRenderer->BindTexture(nullptr);
    g_theRenderer->BindShader(g_theRenderer->CreateOrGetShaderFromFile("Default", eVertexType::VERTEX_PCU));

    g_theRenderer->DrawVertexArray(static_cast<int>(verts.size()), verts.data());
}

//----------------------------------------------------------------------------------------------------
Mat44 Actor::GetModelToWorldTransform() const
{
    Mat44 m2w;

    m2w.SetTranslation3D(m_position);
    m2w.Append(m_orientation.GetAsMatrix_IFwd_JLeft_KUp());

    return m2w;
}

void Actor::UpdatePhysics(float const deltaSeconds)
{
    float dragValue = m_definition->m_drag;
    Vec3  dragForce = -m_velocity * dragValue;
    AddForce(dragForce);

    m_velocity += m_acceleration * deltaSeconds;
    m_position += m_velocity * deltaSeconds;
    m_acceleration = Vec3::ZERO;
}

void Actor::Damage(int const          damage,
                   ActorHandle const& other)
{
    m_health -= damage;

    if (m_health < 0)
    {
        m_dead = true;
    }

    if (m_aiController != nullptr)
    {
        m_aiController->DamagedBy(other);
    }
}

void Actor::AddForce(Vec3 const& force)
{
    m_acceleration += force;
}

void Actor::AddImpulse(Vec3 const& impulse)
{
    m_velocity += impulse;
}

void Actor::MoveInDirection(Vec3 const& direction,
                            float const speed)
{
    Vec3  directionNormal = direction.GetNormalized();
    float dragValue       = m_definition->m_drag;
    Vec3  force           = directionNormal * speed * dragValue;
    AddForce(force);
}

void Actor::TurnInDirection(EulerAngles const& direction)
{
    m_orientation = direction;

    // DebuggerPrintf("Actor Orientation(%f, %f, %f)\n", m_orientation.m_yawDegrees, m_orientation.m_pitchDegrees, m_orientation.m_rollDegrees);
}

//----------------------------------------------------------------------------------------------------
void Actor::OnPossessed(Controller* controller)
{
    m_controller = controller;
    // m_isVisible  = !m_isVisible;
    m_isVisible = false;
}

//----------------------------------------------------------------------------------------------------
// a. Actors remember who their controller is.
// b. If the player possesses an actor with an AI controller, the AI controller is saved and then restored if the player unpossesses the actor.
void Actor::OnUnpossessed()
{
    m_controller = nullptr;
    m_isVisible  = true;

    if (m_aiController == nullptr) return;
    m_controller = m_aiController;
    // m_aiController->Possess(m_handle);
}

void Actor::OnCollisionEnterWithActor(Actor* other)
{
    if (m_isDead || other->m_isDead) return;
    if (m_owner && other->m_owner) return;

    if (this == other) return;

    Vec2 positionXY       = Vec2(m_position.x, m_position.y);
    Vec2 otherPositionXY  = Vec2(other->m_position.x, other->m_position.y);
    Vec2 actorAPositionXY = Vec2(m_position.x, m_position.y);

    if (DoDiscsOverlap2D(positionXY, m_radius, otherPositionXY, other->m_radius))
    {
        if (m_definition->m_name == "PlasmaProjectile" && other->m_definition->m_name == "Demon")
        {
            other->Damage(10, m_owner->m_handle);
            Vec3 forward, left, right;
            m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, right);
            other->AddImpulse(forward);
            m_isDead = true;
        }
        // other->Damage(10,m_handle);
        // Vec3 forward, left, right;
        // other->m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, right);
        // if (!m_owner && other->m_owner)
        // {
        //     if (m_isDead == true)return;
        //     Damage(10, m_handle);
        //     Vec3 forward, left, right;
        //     other->m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, right);
        //     m_isDead = true;
        // }
    }

    Vec2        actorBPositionXY = Vec2(other->m_position.x, other->m_position.y);
    float const actorARadius     = m_radius;
    float const actorBRadius     = other->m_radius;

    // 5. Push movable actor out of immovable actor.
    PushDiscsOutOfEachOther2D(actorAPositionXY, actorARadius, actorBPositionXY, actorBRadius);

    // 6. Update actors' position.
    m_position.x        = actorAPositionXY.x;
    m_position.y        = actorAPositionXY.y;
    other->m_position.x = actorBPositionXY.x;
    other->m_position.y = actorBPositionXY.y;
}

void Actor::OnCollisionEnterWithMap(Actor*         other,
                                    IntVec2 const& tileCoords)
{
    // TODO: Swap check method for Sprinting if needed (PushCapsuleOutOfAABB2D/DoCapsuleAndAABB2Overlap2D)

    AABB3 const aabb3Box = other->m_map->GetTile(tileCoords.x, tileCoords.y)->m_bounds;
    AABB2 const aabb2Box = AABB2(Vec2(aabb3Box.m_mins.x, aabb3Box.m_mins.y), Vec2(aabb3Box.m_maxs.x, aabb3Box.m_maxs.y));

    Vec2 actorPositionXY = Vec2(other->m_position.x, other->m_position.y);

    PushDiscOutOfAABB2D(actorPositionXY, other->m_radius, aabb2Box);

    other->m_position.x = actorPositionXY.x;
    other->m_position.y = actorPositionXY.y;
}

void Actor::Attack()
{
    if (m_currentWeapon == nullptr) return;
    m_currentWeapon->Fire();
}

Vec3 Actor::GetActorEyePosition() const
{
    return m_position + Vec3(0.f, 0.f, m_definition->m_eyeHeight);
}
