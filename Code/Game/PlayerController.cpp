//----------------------------------------------------------------------------------------------------
// Player.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/PlayerController.hpp"

#include "Actor.hpp"
#include "ActorDefinition.hpp"
#include "Game.hpp"
#include "GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Game/Map.hpp"

//----------------------------------------------------------------------------------------------------
PlayerController::PlayerController(Map* owner)
    : Controller(owner)
{
    // ActorDefinition const* definition = ActorDefinition::GetDefByName("Marine");
    //
    // m_eyeHeight = definition->m_eyeHeight;
    // m_cameraFOV = definition->m_cameraFOV;

    m_worldCamera = new Camera();
    // m_worldCamera->SetPerspectiveGraphicView(2.f, 60.f, 0.1f, 100.f);
    // m_worldCamera->SetPosition(Vec3(-2.f, 0.f, 0.f));

    // m_position = Vec3(2.5f, 8.5f, 0.5f);
}

//----------------------------------------------------------------------------------------------------
PlayerController::~PlayerController()
{
    if (m_worldCamera != nullptr)
    {
        delete m_worldCamera;
        m_worldCamera = nullptr;
    }
}

//----------------------------------------------------------------------------------------------------
void PlayerController::Update(float deltaSeconds)
{
    XboxController const& controller = g_theInput->GetController(0);

    UpdateFromInput();
    Vec2 const cursorClientDelta = g_theInput->GetCursorClientDelta();

    // if playerController is possessing a valid actor
    if (!m_actorHandle.IsValid()) return;

    // if playerController is not camera mode
    if (!m_isCameraMode)
    {
        Actor* possessedActor = GetActor();

        if (possessedActor == nullptr) return;

        EulerAngles possessedActorOrientation = possessedActor->m_orientation;

        possessedActorOrientation.m_yawDegrees -= cursorClientDelta.x * 0.125f;
        possessedActorOrientation.m_yawDegrees -= cursorClientDelta.x * 0.125f;
        possessedActorOrientation.m_pitchDegrees += cursorClientDelta.y * 0.125f;

        Vec3 forward;
        Vec3 left;
        Vec3 up;
        possessedActorOrientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);

        possessedActor->TurnInDirection(possessedActorOrientation);

        if (g_theInput->IsKeyDown(KEYCODE_W))
        {
            possessedActor->MoveInDirection(forward, 1.5f);
        }

        if (g_theInput->IsKeyDown(KEYCODE_S))
        {
            possessedActor->MoveInDirection(-forward, 1.5f);
        }

        if (g_theInput->IsKeyDown(KEYCODE_A))
        {
            possessedActor->MoveInDirection(left, 1.5f);
        }

        if (g_theInput->IsKeyDown(KEYCODE_D))
        {
            possessedActor->MoveInDirection(-left, 1.5f);
        }
    }
    else
    {
        Vec3 forward;
        Vec3 left;
        Vec3 up;
        m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);

        m_velocity                = Vec3::ZERO;
        float constexpr moveSpeed = 2.f;

        Vec2 const leftStickInput = controller.GetLeftStick().GetPosition();
        m_velocity += Vec3(leftStickInput.y, -leftStickInput.x, 0.f) * moveSpeed;

        if (g_theInput->IsKeyDown(KEYCODE_W)) m_velocity += forward * moveSpeed;
        if (g_theInput->IsKeyDown(KEYCODE_S)) m_velocity -= forward * moveSpeed;
        if (g_theInput->IsKeyDown(KEYCODE_A)) m_velocity += left * moveSpeed;
        if (g_theInput->IsKeyDown(KEYCODE_D)) m_velocity -= left * moveSpeed;
        if (g_theInput->IsKeyDown(KEYCODE_Z) || controller.IsButtonDown(XBOX_BUTTON_LSHOULDER)) m_velocity -= Vec3(0.f, 0.f, 1.f) * moveSpeed;
        if (g_theInput->IsKeyDown(KEYCODE_C) || controller.IsButtonDown(XBOX_BUTTON_RSHOULDER)) m_velocity += Vec3(0.f, 0.f, 1.f) * moveSpeed;

        if (g_theInput->IsKeyDown(KEYCODE_SHIFT) || controller.IsButtonDown(XBOX_BUTTON_A)) deltaSeconds *= 10.f;

        m_position += m_velocity * deltaSeconds;

        Vec2 const rightStickInput = controller.GetRightStick().GetPosition();
        m_orientation.m_yawDegrees -= rightStickInput.x * 0.125f;
        m_orientation.m_pitchDegrees -= rightStickInput.y * 0.125f;

        m_orientation.m_yawDegrees -= g_theInput->GetCursorClientDelta().x * 0.125f;
        m_orientation.m_pitchDegrees += g_theInput->GetCursorClientDelta().y * 0.125f;
        m_orientation.m_pitchDegrees = GetClamped(m_orientation.m_pitchDegrees, -85.f, 85.f);


        float const leftTriggerInput  = controller.GetLeftTrigger();
        float const rightTriggerInput = controller.GetRightTrigger();

        if (leftTriggerInput != 0.f)
        {
            m_orientation.m_rollDegrees -= 90.f;
        }

        if (rightTriggerInput != 0.f)
        {
            m_orientation.m_rollDegrees += 90.f;
        }

        if (g_theInput->IsKeyDown(KEYCODE_Q)) m_orientation.m_rollDegrees = 90.f;
        if (g_theInput->IsKeyDown(KEYCODE_E)) m_orientation.m_rollDegrees = -90.f;

        m_orientation.m_rollDegrees = GetClamped(m_orientation.m_rollDegrees, -45.f, 45.f);
    }

    UpdateWorldCamera();
}

//----------------------------------------------------------------------------------------------------
void PlayerController::Render() const
{
}

//----------------------------------------------------------------------------------------------------
void PlayerController::UpdateFromInput()
{
    if (g_theInput->WasKeyJustPressed(KEYCODE_N))
    {
        m_map->DebugPossessNext();
        m_isCameraMode = false;
    }
    if (g_theInput->WasKeyJustPressed(KEYCODE_F))
    {
        m_isCameraMode = !m_isCameraMode;

        Actor* possessedActor = GetActor();

        if (possessedActor == nullptr) return;

        possessedActor->m_isVisible = !possessedActor->m_isVisible;
        DebuggerPrintf("PC: %d, %d\n", possessedActor->m_handle.GetIndex(), possessedActor->m_isVisible);
    }

    if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
    {
        Vec3 const forwardNormal = m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D().GetNormalized();
        Ray3 const ray           = Ray3(m_position, m_position + forwardNormal * 10.f);
        // RaycastResult3D const result        = m_game->GetCurrentMap()->RaycastAll(m_position, forwardNormal, ray.m_maxLength);
        RaycastResult3D const result = m_map->RaycastAll(m_position, forwardNormal, ray.m_maxLength);

        if (result.m_didImpact == true)
        {
            DebugAddWorldLine(ray.m_startPosition, result.m_impactPosition, 0.01f, 10.f);
            DebugAddWorldPoint(result.m_impactPosition, 0.06f, 10.f);
            DebugAddWorldArrow(result.m_impactPosition, result.m_impactPosition + result.m_impactNormal * 0.3f, 0.03f, 10.f, Rgba8::BLUE, Rgba8::BLUE);
            DebugAddWorldLine(result.m_impactPosition, ray.m_startPosition + ray.m_forwardNormal * ray.m_maxLength, 0.01f, 10.f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::X_RAY);
        }
        else
        {
            DebugAddWorldLine(ray.m_startPosition, ray.m_startPosition + ray.m_forwardNormal * ray.m_maxLength, 0.01f, 10.f);
        }
    }
}

//----------------------------------------------------------------------------------------------------
void PlayerController::UpdateWorldCamera()
{
    if (g_theGame->GetGameState() != eGameState::INGAME) return;

    if (!m_isCameraMode)
    {
        Actor const* possessedActor = GetActor();
        // if (possessActor && !possessActor->m_bIsDead)
        if (possessedActor != nullptr)
        {
            m_eyeHeight = possessedActor->m_definition->m_eyeHeight;
            m_cameraFOV = possessedActor->m_definition->m_cameraFOV;
            m_worldCamera->SetPerspectiveGraphicView(2.0f, m_cameraFOV, 0.1f, 100.f);
            // Set the world camera to use the possessed actor's eye height and FOV.
            m_position = Vec3(possessedActor->m_position.x, possessedActor->m_position.y, m_eyeHeight);
            // m_position += Vec3::X_BASIS;
            m_orientation = possessedActor->m_orientation;
        }
        else
        {
            m_worldCamera->SetPerspectiveGraphicView(2.0f, m_cameraFOV, 0.1f, 100.f);
        }
    }

    m_worldCamera->SetPositionAndOrientation(m_position, m_orientation);
    Mat44 c2r;
    c2r.SetIJK3D(Vec3::Z_BASIS, -Vec3::X_BASIS, Vec3::Y_BASIS);
    m_worldCamera->SetCameraToRenderTransform(c2r);
}

//----------------------------------------------------------------------------------------------------
void PlayerController::UpdateFromController()
{
}

//----------------------------------------------------------------------------------------------------
Camera* PlayerController::GetCamera() const
{
    return m_worldCamera;
}

Mat44 PlayerController::GetModelToWorldTransform() const
{
    Mat44 m2w;

    m2w.SetTranslation3D(m_position);

    m2w.AppendZRotation(m_orientation.m_yawDegrees);
    m2w.AppendYRotation(m_orientation.m_pitchDegrees);
    m2w.AppendXRotation(m_orientation.m_rollDegrees);

    // m2w.Append(m_orientation.GetAsMatrix_IFwd_JLeft_KUp());

    return m2w;
}
