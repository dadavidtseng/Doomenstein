//----------------------------------------------------------------------------------------------------
// PlayerController.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Framework/PlayerController.hpp"

#include "Game/Gameplay/Weapon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Platform/Window.hpp"
#include "Game/Gameplay/Actor.hpp"
#include "Game/Definition/ActorDefinition.hpp"
#include "Game/Gameplay/Game.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Gameplay/Map.hpp"

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
    m_speed    = g_gameConfigBlackboard.GetValue("playerSpeed", m_speed);
    m_turnRate = g_gameConfigBlackboard.GetValue("playerTurnRate", m_turnRate);
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
    GAME_SAFE_RELEASE(m_worldCamera);
}

//----------------------------------------------------------------------------------------------------
void PlayerController::Update(float const deltaSeconds)
{
    UpdateInput(deltaSeconds);

    UpdateWorldCamera();
}

void PlayerController::UpdateInput(float const deltaSeconds)
{
    UNUSED(deltaSeconds)

    switch (m_deviceType)
    {
    case eDeviceType::CONTROLLER:
        {
            UpdateFromController(deltaSeconds);
            break;
        }
    case eDeviceType::KEYBOARD_AND_MOUSE:
        {
            UpdateFromKeyboard(deltaSeconds);
            break;
        }
    }
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
    if (possessActor&&possessActor->m_definition->m_name == "Marine")
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
void PlayerController::UpdateFromKeyboard(float deltaSeconds)
{
    if (m_deviceType == eDeviceType::CONTROLLER) return;
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

        // possessedActor->m_isVisible = !possessedActor->m_isVisible;
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

        // Vec2 const leftStickInput = controller.GetLeftStick().GetPosition();
        // m_velocity += Vec3(leftStickInput.y, -leftStickInput.x, 0.f) * moveSpeed;

        if (g_theInput->IsKeyDown(KEYCODE_W)) m_velocity += forward * moveSpeed;
        if (g_theInput->IsKeyDown(KEYCODE_S)) m_velocity -= forward * moveSpeed;
        if (g_theInput->IsKeyDown(KEYCODE_A)) m_velocity += left * moveSpeed;
        if (g_theInput->IsKeyDown(KEYCODE_D)) m_velocity -= left * moveSpeed;
        if (g_theInput->IsKeyDown(KEYCODE_Z)) m_velocity -= Vec3(0.f, 0.f, 1.f) * moveSpeed;
        if (g_theInput->IsKeyDown(KEYCODE_C)) m_velocity += Vec3(0.f, 0.f, 1.f) * moveSpeed;

        if (g_theInput->IsKeyDown(KEYCODE_SHIFT)) deltaSeconds *= 10.f;

        m_position += m_velocity * deltaSeconds;

        m_orientation.m_yawDegrees -= cursorClientDelta.x * 0.125f;
        m_orientation.m_pitchDegrees += cursorClientDelta.y * 0.125f;
        m_orientation.m_pitchDegrees = GetClamped(m_orientation.m_pitchDegrees, -85.f, 85.f);


        // if (g_theInput->IsKeyDown(KEYCODE_Q)) m_orientation.m_rollDegrees = 90.f;
        // if (g_theInput->IsKeyDown(KEYCODE_E)) m_orientation.m_rollDegrees = -90.f;

        m_orientation.m_rollDegrees = GetClamped(m_orientation.m_rollDegrees, -45.f, 45.f);
    }
}

void PlayerController::UpdateFromController(float deltaSeconds)
{
    if (g_theGame->GetGameState() != eGameState::INGAME)
    return;

const XboxController& controller = g_theInput->GetController(0);
float                 speed      = m_speed;
float                 turnRate   = m_turnRate;
if (!m_isCameraMode)
{
    Actor* possessActor = GetActor();
    if (!possessActor)
        return;
    if (possessActor->m_isDead) /// Handle player Actor dead
        return;
    EulerAngles possessActorOrientation = possessActor->m_orientation;

    float actorSpeed = possessActor->m_definition->m_walkSpeed;

    Vec2  leftStickPos  = controller.GetLeftStick().GetPosition();
    Vec2  rightStickPos = controller.GetRightStick().GetPosition();
    float leftStickMag  = controller.GetLeftStick().GetMagnitude();
    float rightStickMag = controller.GetRightStick().GetMagnitude();
    float leftTrigger   = controller.GetLeftTrigger();
    float rightTrigger  = controller.GetRightTrigger();


    if (rightStickMag > 0.f)
    {
        turnRate = possessActor->m_definition->m_turnSpeed;
        possessActorOrientation.m_yawDegrees += -(rightStickPos * speed * rightStickMag * turnRate * deltaSeconds).x;
        possessActorOrientation.m_pitchDegrees += -(rightStickPos * speed * rightStickMag * turnRate * deltaSeconds).y;
        possessActor->TurnInDirection(possessActorOrientation);
    }

    if (controller.IsButtonDown(XBOX_BUTTON_A))
    {
        actorSpeed = possessActor->m_definition->m_runSpeed;
    }

    Vec3 forward, left, up;
    possessActor->m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
    if (leftStickMag > 0.f)
    {
        // Combine X / Y stick input into one movement vector
        Vec3 moveDir = forward * leftStickPos.y + -left * leftStickPos.x;
        moveDir.z    = 0.f;
        possessActor->MoveInDirection(moveDir.GetNormalized(), actorSpeed);
        possessActor->PlayAnimationByName("Walk");
    }

    if (controller.WasButtonJustPressed(XBOX_BUTTON_DPAD_DOWN))
    {
        auto it    = std::find(possessActor->m_weapons.begin(), possessActor->m_weapons.end(), possessActor->m_currentWeapon);
        int  index = static_cast<int>(it - possessActor->m_weapons.begin());
        index--;
        int newIndex = static_cast<int>((index + possessActor->m_weapons.size()) % possessActor->m_weapons.size());
        possessActor->SwitchInventory(newIndex);
    }
    if (rightTrigger > 0.f)
    {
        GetActor()->m_currentWeapon->Fire();
    }
    if (controller.WasButtonJustPressed(XBOX_BUTTON_Y))
    {
        possessActor->SwitchInventory(1);
    }
    if (controller.WasButtonJustPressed(XBOX_BUTTON_X))
    {
        possessActor->SwitchInventory(0);
    }
    if (controller.WasButtonJustPressed(XBOX_BUTTON_DPAD_UP))
    {
        auto it    = std::find(possessActor->m_weapons.begin(), possessActor->m_weapons.end(), possessActor->m_currentWeapon);
        int  index = static_cast<int>(it - possessActor->m_weapons.begin());
        index++;
        int newIndex = index % possessActor->m_weapons.size();
        possessActor->SwitchInventory(newIndex);
    }

    m_orientation.m_rollDegrees += leftTrigger * turnRate * deltaSeconds * speed;
    m_orientation.m_rollDegrees -= rightTrigger * turnRate * deltaSeconds * speed;
}
}

//----------------------------------------------------------------------------------------------------
void PlayerController::UpdateWorldCamera()
{
    if (g_theGame->GetGameState() != eGameState::INGAME) return;

    Actor const* possessedActor = GetActor();

    if (!m_isCameraMode)
    {
        // if (possessActor && !possessActor->m_isDead)
        if (possessedActor != nullptr&&!possessedActor->m_isDead)
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
