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

    m_health      = m_definition->m_health;
    m_height      = m_definition->m_height;
    m_radius      = m_definition->m_radius;
    m_position    = spawnInfo.m_position;
    m_orientation = spawnInfo.m_orientation;

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
void Actor::Update()
{
    if (!m_isMovable) return;

    if (!dynamic_cast<PlayerController*>(m_controller)->m_isCameraMode)
    {
        UpdatePosition();
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
    VertexList_PCU verts;

    AddVertsForCylinder3D(verts, m_collisionCylinder.m_startPosition, m_collisionCylinder.m_endPosition, m_collisionCylinder.m_radius, m_color);
    AddVertsForWireframeCylinder3D(verts, m_collisionCylinder.m_startPosition, m_collisionCylinder.m_endPosition, m_collisionCylinder.m_radius, 0.001f);

    g_theRenderer->SetModelConstants();
    g_theRenderer->SetBlendMode(eBlendMode::OPAQUE);
    g_theRenderer->SetRasterizerMode(eRasterizerMode::SOLID_CULL_NONE);
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

//----------------------------------------------------------------------------------------------------
void Actor::OnPossessed(Controller* controller)
{
    m_controller                       = controller;
    PlayerController* playerController = dynamic_cast<PlayerController*>(m_controller);
    playerController->m_position       = m_position + Vec3(1.f, 0.f, playerController->m_eyeHeight);
}

//----------------------------------------------------------------------------------------------------
void Actor::OnUnpossessed()
{
    m_controller = nullptr;

    if (m_aiController != nullptr)
    {
        m_aiController->Possess(m_handle);
    }
}
