//----------------------------------------------------------------------------------------------------
// Actor.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Actor.hpp"

#include "ActorDefinition.hpp"
#include "AIController.hpp"
#include "Game.hpp"
#include "GameCommon.hpp"
#include "MapDefinition.hpp"
#include "PlayerController.hpp"
#include "Weapon.hpp"
#include "WeaponDefinition.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
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


    m_collisionCylinder = Cylinder3(m_position, m_position + Vec3(0.f, 0.f, m_height), m_radius);
}

//----------------------------------------------------------------------------------------------------
void Actor::Update(float const deltaSeconds)
{
    // if (!m_isMovable) return;

    PlayerController const* playerController = dynamic_cast<PlayerController*>(m_controller);
    if (playerController != nullptr &&
        !playerController->m_isCameraMode)
    {
        // UpdatePosition();
        UpdatePhysics(deltaSeconds);
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
    // if (dynamic_cast<PlayerController*>(m_controller) != nullptr && !m_isVisible) return;
    if (!m_isVisible) return;

    VertexList_PCU verts;
    float const    eyeHeight         = m_definition->m_eyeHeight;
    Vec3 const     forwardNormal     = m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D().GetNormalized();
    Vec3 const     coneStartPosition = m_collisionCylinder.m_startPosition + Vec3(0.f, 0.f, eyeHeight) + forwardNormal * m_radius;
    AddVertsForCone3D(verts, coneStartPosition, coneStartPosition + forwardNormal * 0.1f, 0.1f, m_color);
    AddVertsForWireframeCone3D(verts, coneStartPosition, coneStartPosition + forwardNormal * 0.1f, 0.1f, 0.001f);
    AddVertsForCylinder3D(verts, m_collisionCylinder.m_startPosition, m_collisionCylinder.m_endPosition, m_collisionCylinder.m_radius, m_color);
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
    // DebuggerPrintf("%s, %f, %f, %f\n", m_definition->m_name.c_str(), m_position.x, m_position.y, m_position.z);
    // DebuggerPrintf("%f, %f, %f\n", m_velocity.x, m_velocity.y, m_velocity.z);

    float dragValue = m_definition->m_drag;
    Vec3  dragForce = -m_velocity * dragValue;
    AddForce(dragForce);

    m_velocity += m_acceleration * deltaSeconds;
    m_position += m_velocity * deltaSeconds;
    m_acceleration = Vec3::ZERO;
    // DebuggerPrintf("%s, %f, %f, %f\n", m_definition->m_name.c_str(), m_position.x, m_position.y, m_position.z);
    // DebuggerPrintf("%f, %f, %f\n", m_velocity.x, m_velocity.y, m_velocity.z);
}

void Actor::Damage(float const damage,
                   ActorHandle other)
{
    m_health -= damage;
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

void Actor::Attack()
{
    if (m_currentWeapon == nullptr) return;
    m_currentWeapon->Fire();
}
