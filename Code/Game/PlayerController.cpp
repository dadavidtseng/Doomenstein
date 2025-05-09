//----------------------------------------------------------------------------------------------------
// PlayerController.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/PlayerController.hpp"

#include "Weapon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Game/Actor.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"

//----------------------------------------------------------------------------------------------------
PlayerController::PlayerController(Map* owner)
    : Controller(owner)
{
    // m_worldCamera = new Camera();
    m_worldCamera         = new Camera();
    m_worldCamera->m_mode = Camera::eMode_Perspective;
    m_worldCamera->SetOrthoGraphicView(Vec2(-1, -1), Vec2(1, 1));
    m_viewCamera         = new Camera();
    m_viewCamera->m_mode = Camera::eMode_Orthographic;
    m_viewCamera->SetOrthoGraphicView(Vec2::ZERO, g_theGame->m_screenSpace.m_maxs); // TODO: use the normalized viewport
    // m_viewCamera->SetOrthoGraphicView(Vec2(0.f, 400.f), Vec2(1600.f, 800.f));
    // m_worldCamera->m_viewPort = AABB2(Vec2(0.f, 0.5f), Vec2::ONE);
    // m_worldCamera->m_viewPort = AABB2(Vec2(0.f, 800.f), Vec2(1600.f,0.f));
    // m_viewCamera->m_viewPort = AABB2(Vec2(0.f, 0.f), Vec2(1600.f,400.f));
// m_worldCamera->SetNormalizedViewport(AABB2(Vec2::ZERO, Vec2::ONE));
// m_viewCamera->SetNormalizedViewport(AABB2(Vec2::ZERO, Vec2::ONE));
// SetViewport(AABB2(Vec2::ZERO, Vec2(1.f, 1.f)));
    // m_viewCamera->SetOrthoGraphicView(Vec2(-800.f, -400.f), g_theGame->m_screenSpace.m_maxs); // TODO: use the normalized viewport
}

//----------------------------------------------------------------------------------------------------
PlayerController::~PlayerController()
{
    SafeDeletePointer(m_worldCamera);
}

//----------------------------------------------------------------------------------------------------
void PlayerController::Update(float deltaSeconds)
{
    XboxController const& controller = g_theInput->GetController(0);
    UpdateFromInput();
    Vec2 const cursorClientDelta = g_theInput->GetCursorClientDelta();

    // if playerController is possessing a valid actor
    if (!m_actorHandle.IsValid()) return;
    // if (m_index == 1)
    // {
    // if playerController is not camera mode
    if (!m_isCameraMode)
    {
        Actor* possessedActor = GetActor();

        if (possessedActor == nullptr) return;
        if (possessedActor->m_definition->m_name == "Marine")
        {
            if (possessedActor->m_currentWeapon)
            {
                possessedActor->m_currentWeapon->Update(deltaSeconds);
            }
        }

        EulerAngles possessedActorOrientation = possessedActor->m_orientation;
        float       speed                     = possessedActor->m_definition->m_walkSpeed;

        possessedActorOrientation.m_yawDegrees -= cursorClientDelta.x * 0.125f;
        possessedActorOrientation.m_pitchDegrees += cursorClientDelta.y * 0.125f;
        possessedActorOrientation.m_pitchDegrees = GetClamped(possessedActorOrientation.m_pitchDegrees, -85.f, 85.f);


        Vec3 forward;
        Vec3 left;
        Vec3 up;
        possessedActorOrientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);

        possessedActor->TurnInDirection(possessedActorOrientation);


        if (g_theInput->WasKeyJustPressed(NUMCODE_1))
        {
            possessedActor->SwitchInventory(0);
        }

        if (g_theInput->WasKeyJustPressed(NUMCODE_2))
        {
            possessedActor->SwitchInventory(1);
        }

        if (g_theInput->IsKeyDown(KEYCODE_SHIFT))
        {
            speed = possessedActor->m_definition->m_runSpeed;
        }

        if (g_theInput->IsKeyDown(KEYCODE_W))
        {
            possessedActor->MoveInDirection(forward, speed);
            possessedActor->PlayAnimationByName("Walk");
        }

        if (g_theInput->IsKeyDown(KEYCODE_S))
        {
            possessedActor->MoveInDirection(-forward, speed);
        }

        if (g_theInput->IsKeyDown(KEYCODE_A))
        {
            possessedActor->MoveInDirection(left, speed);
        }

        if (g_theInput->IsKeyDown(KEYCODE_D))
        {
            possessedActor->MoveInDirection(-left, speed);
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

        m_orientation.m_yawDegrees -= cursorClientDelta.x * 0.125f;
        m_orientation.m_pitchDegrees += cursorClientDelta.y * 0.125f;
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

        // if (g_theInput->IsKeyDown(KEYCODE_Q)) m_orientation.m_rollDegrees = 90.f;
        // if (g_theInput->IsKeyDown(KEYCODE_E)) m_orientation.m_rollDegrees = -90.f;

        m_orientation.m_rollDegrees = GetClamped(m_orientation.m_rollDegrees, -45.f, 45.f);
    }
    // }
    UpdateWorldCamera();
}

//----------------------------------------------------------------------------------------------------
void PlayerController::Render() const
{
    // Actor const* possessedActor = m_map->GetActorByHandle(m_actorHandle);
    // if (possessedActor == nullptr) return;
    // String const possessedActorName   = possessedActor->m_definition->m_name;
    // int const    possessedActorHealth = possessedActor->m_health;
    //
    // DebugAddScreenText(Stringf("Name:%s/Health:%d", possessedActorName.c_str(), possessedActorHealth), Vec2(0.f, 20.f), 20.f, Vec2::ZERO, 0.f);
    //
    // if (possessedActor->m_currentWeapon)
    // {
    //     possessedActor->m_currentWeapon->Render();
    // }


    g_theRenderer->BeginCamera(*m_viewCamera);
    if (g_theGame->GetGameState() != eGameState::INGAME) return;
    if (!m_actorHandle.IsValid()) return;
    if (m_isCameraMode) return;
    Actor* possessActor = m_map->GetActorByHandle(m_actorHandle);
    if (possessActor->m_definition->m_name == "Marine")
    {
        if (possessActor->m_currentWeapon) possessActor->m_currentWeapon->Render();
    }
    g_theRenderer->EndCamera(*m_viewCamera);
}

eDeviceType PlayerController::SetInputDeviceType(eDeviceType newDeviceType)
{
    m_deviceType = newDeviceType;
    return newDeviceType;
}

eDeviceType PlayerController::GetInputDeviceType() const
{
    return m_deviceType;
}

//----------------------------------------------------------------------------------------------------
void PlayerController::UpdateFromInput()
{
    if (m_deviceType == eDeviceType::CONTROLLER)return;
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
    }

    if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
    {
        if (m_isCameraMode) return;

        Actor* possessedActor = GetActor();

        if (possessedActor != nullptr)
        {
            possessedActor->Attack();
        }
    }
}

//----------------------------------------------------------------------------------------------------
void PlayerController::UpdateWorldCamera()
{
    if (g_theGame->GetGameState() != eGameState::INGAME) return;
    if (m_deviceType == eDeviceType::CONTROLLER)return;

    Actor const* possessedActor = GetActor();

    if (!m_isCameraMode)
    {
        // if (possessActor && !possessActor->m_isDead)
        if (possessedActor != nullptr)
        {
            // m_worldCamera->SetOrthoGraphicView(Vec2(-1, -1), Vec2(1, 1));
            // m_worldCamera->SetPerspectiveGraphicView(2.0f, possessedActor->m_definition->m_cameraFOV, 0.1f, 100.f);
            m_worldCamera->SetPerspectiveGraphicView(2.0f, possessedActor->m_definition->m_cameraFOV, 0.1f, 100.f);
            // Set the world camera to use the possessed actor's eye height and FOV.
            m_position = Vec3(possessedActor->m_position.x, possessedActor->m_position.y, possessedActor->m_definition->m_eyeHeight);
            // m_position += Vec3::X_BASIS;
            m_orientation = possessedActor->m_orientation;
        }
        else
        {
            m_worldCamera->SetPerspectiveGraphicView(2.0f, possessedActor->m_definition->m_cameraFOV, 0.1f, 100.f);
        }
    }

    if (possessedActor->m_isDead)
    {
        Vec3  startPos      = possessedActor->m_position + Vec3(0.f, 0.f, possessedActor->m_definition->m_eyeHeight);
        Vec3  endPos        = possessedActor->m_position;
        float deathFraction = possessedActor->m_dead / possessedActor->m_definition->m_corpseLifetime;
        float interpolate   = Interpolate(startPos.z, endPos.z, deathFraction);
        m_position          = Vec3(possessedActor->m_position.x, possessedActor->m_position.y, interpolate);
    }

    m_viewCamera->SetOrthoGraphicView(g_theGame->m_screenSpace.m_mins, g_theGame->m_screenSpace.m_maxs);
    m_worldCamera->SetPosition(m_position);
    m_worldCamera->SetOrientation(m_orientation);

    float x = (float)Window::s_mainWindow->GetClientDimensions().x;
    float y = (float)Window::s_mainWindow->GetClientDimensions().y;

    if (m_index == 0)
    {
        // SetViewport(AABB2(Vec2::ZERO, Vec2::ONE));
        // m_worldCamera->m_viewPort = AABB2(Vec2(0.f, 0.f), Vec2(x, y * 0.5f));
        // m_viewCamera->m_viewPort  = AABB2(Vec2(0.f, 0.f), Vec2(x, y * 0.5f));
    }

    if (m_index == 1)
    {
        // SetViewport(AABB2(Vec2::ZERO, Vec2::ONE));

        m_worldCamera->m_viewPort = AABB2(Vec2(0.f, y * 0.5f), Vec2(x, y));
        m_viewCamera->m_viewPort  = AABB2(Vec2(0.f, y * 0.5f), Vec2(x, y));
    }

    Mat44 ndcMatrix;
    ndcMatrix.SetIJK3D(Vec3(0, 0, 1), Vec3(-1, 0, 0), Vec3(0, 1, 0));
    m_worldCamera->SetCameraToRenderTransform(ndcMatrix);
}

Mat44 PlayerController::GetModelToWorldTransform() const
{
    Mat44 m2w;

    m2w.SetTranslation3D(m_position);

    m2w.Append(m_orientation.GetAsMatrix_IFwd_JLeft_KUp());

    return m2w;
}
