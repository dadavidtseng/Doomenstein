//----------------------------------------------------------------------------------------------------
// Game.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Game.hpp"

#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Actor.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/PlayerController.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/WeaponDefinition.hpp"

//----------------------------------------------------------------------------------------------------
Game::Game()
{
    m_screenCamera = new Camera();

    Vec2 const  bottomLeft     = Vec2::ZERO;
    float const screenSizeX    = g_gameConfigBlackboard.GetValue("screenSizeX", -1.f);
    float const screenSizeY    = g_gameConfigBlackboard.GetValue("screenSizeY", -1.f);
    Vec2 const  screenTopRight = Vec2(screenSizeX, screenSizeY);

    m_screenSpace.m_mins = Vec2::ZERO;
    m_screenSpace.m_maxs = Vec2(g_gameConfigBlackboard.GetValue("screenSizeX", 1600.f), g_gameConfigBlackboard.GetValue("screenSizeY", 800.f));
    // m_screenSpace.m_maxs = Vec2(1600.f,400.f);
    m_worldSpace.m_mins = Vec2::ZERO;
    m_worldSpace.m_maxs = Vec2(g_gameConfigBlackboard.GetValue("worldSizeX", 200.f), g_gameConfigBlackboard.GetValue("worldSizeY", 100.f));

    m_screenCamera->SetOrthoGraphicView(bottomLeft, screenTopRight);
    // m_screenCamera->SetOrthoGraphicView(Vec2(0.f,0.f),Vec2(1600.f, 400.f));

    m_screenCamera->SetNormalizedViewport(AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)));

    m_gameClock = new Clock(Clock::GetSystemClock());

    DebugAddWorldBasis(Mat44(), -1.f);

    Mat44 transform;

    transform.SetIJKT3D(-Vec3::Y_BASIS, Vec3::X_BASIS, Vec3::Z_BASIS, Vec3(0.25f, 0.f, 0.25f));
    DebugAddWorldText("X-Forward", transform, 0.25f, Vec2::ONE, -1.f, Rgba8::RED);

    transform.SetIJKT3D(-Vec3::X_BASIS, -Vec3::Y_BASIS, Vec3::Z_BASIS, Vec3(0.f, 0.25f, 0.5f));
    DebugAddWorldText("Y-Left", transform, 0.25f, Vec2::ZERO, -1.f, Rgba8::GREEN);

    transform.SetIJKT3D(-Vec3::X_BASIS, Vec3::Z_BASIS, Vec3::Y_BASIS, Vec3(0.f, -0.25f, 0.25f));
    DebugAddWorldText("Z-Up", transform, 0.25f, Vec2(1.f, 0.f), -1.f, Rgba8::BLUE);

    // InitializeMaps();
}

//----------------------------------------------------------------------------------------------------
Game::~Game()
{
    SafeDeletePointer(m_currentMap);
    SafeDeletePointer(m_gameClock);
    // SafeDeletePointer(m_playerController);
    SafeDeletePointer(m_screenCamera);
}

//----------------------------------------------------------------------------------------------------
// All timers in the game, such as those required by weapons, should use the game clock.
void Game::Update()
{
    float const gameDeltaSeconds = static_cast<float>(m_gameClock->GetDeltaSeconds());

    // #TODO: Select keyboard or controller

    UpdateFromKeyBoard();
    UpdateFromController();
    UpdatePlayerController(gameDeltaSeconds);

    // if (m_currentMap != nullptr &&
    //     m_playerController != nullptr)
    // {
    //     m_currentMap->Update(gameDeltaSeconds);
    // }

    if (m_currentMap != nullptr)
    {
        m_currentMap->Update(gameDeltaSeconds);
    }
}

//----------------------------------------------------------------------------------------------------
void Game::Render() const
{
    //-Start-of-Game-Camera---------------------------------------------------------------------------
    if (m_currentGameState == eGameState::INGAME)
    {
        if (m_currentMap != nullptr)
        {
            if (m_localPlayerControllerList.size() == 1)
            {
                m_localPlayerControllerList[0]->SetViewport(AABB2(Vec2::ZERO, Vec2::ONE));
                m_localPlayerControllerList[0]->Render();
                g_theRenderer->BeginCamera(*m_localPlayerControllerList[0]->m_worldCamera);
                m_currentMap->Render(m_localPlayerControllerList[0]);
                g_theRenderer->EndCamera(*m_localPlayerControllerList[0]->m_worldCamera);
            }

            if (m_localPlayerControllerList.size() == 2)
            {
                m_localPlayerControllerList[0]->SetViewport(AABB2(Vec2(0.f,0.f), Vec2(1.f,0.5f)));
                m_localPlayerControllerList[0]->Render();
                g_theRenderer->BeginCamera(*m_localPlayerControllerList[0]->m_worldCamera);
                m_currentMap->Render(m_localPlayerControllerList[0]);
                g_theRenderer->EndCamera(*m_localPlayerControllerList[0]->m_worldCamera);

                m_localPlayerControllerList[1]->SetViewport(AABB2(Vec2(0.f,0.5f), Vec2(1.f,1.f)));
                m_localPlayerControllerList[1]->Render();
                g_theRenderer->BeginCamera(*m_localPlayerControllerList[1]->m_worldCamera);
                m_currentMap->Render(m_localPlayerControllerList[1]);
                g_theRenderer->EndCamera(*m_localPlayerControllerList[1]->m_worldCamera);
            }
        }
    }

    //-End-of-Game-Camera-----------------------------------------------------------------------------
    //------------------------------------------------------------------------------------------------

    if (m_currentGameState == eGameState::INGAME)
    {
        // if (m_playerController != nullptr)
        // {
        //     DebugRenderWorld(*m_playerController->m_worldCamera);
        // }
    }

    //------------------------------------------------------------------------------------------------
    //-Start-of-Screen-Camera-------------------------------------------------------------------------

    g_theRenderer->BeginCamera(*m_screenCamera);

    if (m_currentGameState == eGameState::ATTRACT)
    {
        RenderAttractMode();
    }

    if (m_currentGameState == eGameState::LOBBY)
    {
        RenderLobby();
    }

    if (m_currentGameState == eGameState::INGAME)
    {
        RenderInGame();
        // RenderPlayerController();
    }

    g_theRenderer->EndCamera(*m_screenCamera);

    //-End-of-Screen-Camera---------------------------------------------------------------------------

    if (m_currentGameState == eGameState::INGAME)
    {
        DebugRenderScreen(*m_screenCamera);
    }
}

//----------------------------------------------------------------------------------------------------
void Game::UpdateFromKeyBoard()
{
    if (m_currentGameState == eGameState::ATTRACT)
    {
        if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
        {
            App::RequestQuit();
        }

        if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
        {
            CreateLocalPlayer(0, eDeviceType::KEYBOARD_AND_MOUSE);
            ChangeState(eGameState::LOBBY);
            return;
        }
    }

    if (m_currentGameState == eGameState::LOBBY)
    {
        if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
        {
            if (m_localPlayerControllerList.size() == 1)
            {
                if (GetControllerByDeviceType(eDeviceType::KEYBOARD_AND_MOUSE) == nullptr)
                {
                    if (m_localPlayerControllerList[0]->m_index == 0)
                    {
                        CreateLocalPlayer(1, eDeviceType::KEYBOARD_AND_MOUSE);
                    }
                    if (m_localPlayerControllerList[0]->m_index == 1)
                    {
                        CreateLocalPlayer(0, eDeviceType::KEYBOARD_AND_MOUSE);
                    }
                    return;
                }

                ChangeState(eGameState::INGAME);
                InitializeMaps();
            }

            if (m_localPlayerControllerList.size() == 2)
            {
                ChangeState(eGameState::INGAME);
                InitializeMaps();
            }
        }

        if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
        {
            PlayerController const* localPlayerController = GetControllerByDeviceType(eDeviceType::KEYBOARD_AND_MOUSE);

            if (localPlayerController != nullptr)
            {
                RemoveLocalPlayer(localPlayerController->m_index);

                if (m_localPlayerControllerList.empty())
                {
                    ChangeState(eGameState::ATTRACT);
                }
            }
        }
    }

    if (m_currentGameState == eGameState::INGAME)
    {
        if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
        {
            ChangeState(eGameState::ATTRACT);

            if (m_currentMap != nullptr)
            {
                delete m_currentMap;
                m_currentMap = nullptr;
            }

            m_maps.clear();
            m_localPlayerControllerList.clear();
        }

        if (g_theInput->WasKeyJustPressed(KEYCODE_P))
        {
            m_gameClock->TogglePause();
        }

        if (g_theInput->WasKeyJustPressed(KEYCODE_O))
        {
            m_gameClock->StepSingleFrame();
        }

        if (g_theInput->IsKeyDown(KEYCODE_T))
        {
            m_gameClock->SetTimeScale(0.1f);
        }

        if (g_theInput->WasKeyJustReleased(KEYCODE_T))
        {
            m_gameClock->SetTimeScale(1.f);
        }
    }
}

//----------------------------------------------------------------------------------------------------
void Game::UpdateFromController()
{
    XboxController const& controller = g_theInput->GetController(0);

    if (!controller.IsConnected()) return;

    if (m_currentGameState == eGameState::ATTRACT)
    {
        if (controller.WasButtonJustPressed(XBOX_BUTTON_BACK))
        {
            App::RequestQuit();
        }

        if (controller.WasButtonJustPressed(XBOX_BUTTON_START))
        {
            CreateLocalPlayer(0, eDeviceType::CONTROLLER);
            ChangeState(eGameState::LOBBY);
            return;
        }
    }

    if (m_currentGameState == eGameState::LOBBY)
    {
        if (controller.WasButtonJustPressed(XBOX_BUTTON_START))
        {
            if (m_localPlayerControllerList.size() == 1)
            {
                if (GetControllerByDeviceType(eDeviceType::CONTROLLER) == nullptr)
                {
                    if (m_localPlayerControllerList[0]->m_index == 0)
                    {
                        CreateLocalPlayer(1, eDeviceType::CONTROLLER);
                    }
                    if (m_localPlayerControllerList[0]->m_index == 1)
                    {
                        CreateLocalPlayer(0, eDeviceType::CONTROLLER);
                    }

                    return;
                }

                ChangeState(eGameState::INGAME);
                InitializeMaps();
            }

            if (m_localPlayerControllerList.size() == 2)
            {
                ChangeState(eGameState::INGAME);
                InitializeMaps();
            }
        }

        if (controller.WasButtonJustPressed(XBOX_BUTTON_BACK))
        {
            PlayerController* localPlayerController = GetControllerByDeviceType(eDeviceType::CONTROLLER);

            if (localPlayerController != nullptr)
            {
                RemoveLocalPlayer(localPlayerController->m_index);

                if (m_localPlayerControllerList.empty())
                {
                    ChangeState(eGameState::ATTRACT);
                }
            }
        }
    }

    if (m_currentGameState == eGameState::INGAME)
    {
        if (controller.WasButtonJustPressed(XBOX_BUTTON_BACK))
        {
            ChangeState(eGameState::ATTRACT);

            if (m_currentMap != nullptr)
            {
                delete m_currentMap;
                m_currentMap = nullptr;
            }

            m_maps.clear();
            m_localPlayerControllerList.clear();
        }

        if (controller.WasButtonJustPressed(XBOX_BUTTON_BACK))
        {
            m_currentGameState = eGameState::ATTRACT;
        }

        if (controller.WasButtonJustPressed(XBOX_BUTTON_B))
        {
            m_gameClock->TogglePause();
        }

        if (controller.WasButtonJustPressed(XBOX_BUTTON_Y))
        {
            m_gameClock->StepSingleFrame();
        }

        if (controller.WasButtonJustPressed(XBOX_BUTTON_X))
        {
            m_gameClock->SetTimeScale(0.1f);
        }

        if (controller.WasButtonJustReleased(XBOX_BUTTON_X))
        {
            m_gameClock->SetTimeScale(1.f);
        }
    }
}

//----------------------------------------------------------------------------------------------------
void Game::UpdatePlayerController(float const deltaSeconds) const
{
    // if (m_playerController != nullptr &&
    //     m_currentGameState == eGameState::INGAME)
    // {
    //     float systemDeltaSeconds = (float)Clock::GetSystemClock().GetDeltaSeconds();
    //     m_playerController->Update(systemDeltaSeconds);
    // }

    DebugAddScreenText(Stringf("Time: %.2f\nFPS: %.2f\nScale: %.1f", m_gameClock->GetTotalSeconds(), 1.f / deltaSeconds, m_gameClock->GetTimeScale()), m_screenCamera->GetOrthographicTopRight() - Vec2(250.f, 60.f), 20.f, Vec2::ZERO, 0.f, Rgba8::WHITE, Rgba8::WHITE);

    /// PlayerController
    if (m_currentGameState == eGameState::INGAME)
    {
        for (PlayerController* controller : m_localPlayerControllerList)
        {
            controller->Update((float)Clock::GetSystemClock().GetDeltaSeconds());
            DebugAddMessage(Stringf("PlayerController position: %.2f, %.2f, %.2f", controller->m_position.x, controller->m_position.y, controller->m_position.z), 0);
            DebugAddMessage(Stringf("PlayerController orientation: %.2f, %.2f, %.2f", controller->m_orientation.m_yawDegrees, controller->m_orientation.m_pitchDegrees,
                                    controller->m_orientation.m_rollDegrees),
                            0);
        }
    }
}

//----------------------------------------------------------------------------------------------------
void Game::RenderAttractMode() const
{
    VertexList_PCU verts;
    AddVertsForAABB2D(verts, m_screenSpace.m_mins, m_screenSpace.m_maxs, Rgba8::BLACK);
    AddVertsForDisc2D(verts, m_screenSpace.GetCenter(), 150.f, 10.f, Rgba8::YELLOW);

    g_theRenderer->SetModelConstants();
    g_theRenderer->SetBlendMode(eBlendMode::ALPHA);
    g_theRenderer->SetRasterizerMode(eRasterizerMode::SOLID_CULL_NONE);
    g_theRenderer->SetSamplerMode(eSamplerMode::POINT_CLAMP);
    g_theRenderer->SetDepthMode(eDepthMode::DISABLED);
    g_theRenderer->BindTexture(nullptr);
    g_theRenderer->DrawVertexArray(static_cast<int>(verts.size()), verts.data());

    VertexList_PCU  textVerts;
    float constexpr textHeight = 20.f;
    AABB2 const     textBounds = AABB2(m_screenSpace.m_mins, Vec2(m_screenSpace.m_maxs.x, textHeight * 3.f));
    g_theBitmapFont->AddVertsForTextInBox2D(textVerts, "Press SPACE to join with mouse and keyboard\nPress START to join with controller\nPress ESCAPE or BACK to exit", textBounds, textHeight * 3.f, Rgba8::WHITE, 1.f, Vec2::HALF);
    g_theRenderer->SetModelConstants();
    g_theRenderer->SetBlendMode(eBlendMode::ALPHA);
    g_theRenderer->SetRasterizerMode(eRasterizerMode::SOLID_CULL_NONE);
    g_theRenderer->SetSamplerMode(eSamplerMode::POINT_CLAMP);
    g_theRenderer->SetDepthMode(eDepthMode::DISABLED);
    g_theRenderer->BindTexture(&g_theBitmapFont->GetTexture());
    g_theRenderer->DrawVertexArray(static_cast<int>(textVerts.size()), textVerts.data());
}

void Game::RenderLobby() const
{
    VertexList_PCU verts;

    if (m_localPlayerControllerList.size() == 1)
    {
        m_localPlayerControllerList[0]->SetViewport(AABB2(Vec2::ZERO, Vec2::ONE));

        AddVertsForAABB2D(verts, m_screenSpace.m_mins, m_screenSpace.m_maxs, Rgba8::BLACK);
        AddVertsForDisc2D(verts, m_screenSpace.GetCenter(), 150.f, 10.f, Rgba8::GREEN);

        g_theRenderer->SetModelConstants();
        g_theRenderer->SetBlendMode(eBlendMode::ALPHA);
        g_theRenderer->SetRasterizerMode(eRasterizerMode::SOLID_CULL_NONE);
        g_theRenderer->SetSamplerMode(eSamplerMode::POINT_CLAMP);
        g_theRenderer->SetDepthMode(eDepthMode::DISABLED);
        g_theRenderer->BindTexture(nullptr);
        g_theRenderer->DrawVertexArray(static_cast<int>(verts.size()), verts.data());

        VertexList_PCU  textVerts;
        float constexpr textHeight = 20.f;
        AABB2 const     textBounds = AABB2(m_screenSpace.m_mins, Vec2(m_screenSpace.m_maxs.x, textHeight * 5.f));
        String const    text       = m_localPlayerControllerList[0]->m_deviceType == eDeviceType::KEYBOARD_AND_MOUSE
                                ? "Keyboard and Mouse\nPress SPACE to start game\nPress ESCAPE to leave game\nPress START to join player"
                                : "Controller\nPress START to start game\nPress BACK to leave game\nPress SPACE to join player";

        g_theBitmapFont->AddVertsForTextInBox2D(textVerts, "Player 01\n" + text, textBounds, textHeight * 5.f, Rgba8::WHITE, 1.f, Vec2::HALF);
        g_theRenderer->SetModelConstants();
        g_theRenderer->SetBlendMode(eBlendMode::ALPHA);
        g_theRenderer->SetRasterizerMode(eRasterizerMode::SOLID_CULL_NONE);
        g_theRenderer->SetSamplerMode(eSamplerMode::POINT_CLAMP);
        g_theRenderer->SetDepthMode(eDepthMode::DISABLED);
        g_theRenderer->BindTexture(&g_theBitmapFont->GetTexture());
        g_theRenderer->DrawVertexArray(static_cast<int>(textVerts.size()), textVerts.data());
    }
    else if (m_localPlayerControllerList.size() == 2)
    {
        m_localPlayerControllerList[0]->SetViewport(AABB2(Vec2(0.f, 0.5f), Vec2(1.f, 1.f)));
        m_localPlayerControllerList[1]->SetViewport(AABB2(Vec2(0.f, 0.f), Vec2(1.f, 0.5f)));

        AABB2 const playerController1 = AABB2(Vec2(m_screenSpace.m_mins.x, (float)m_screenSpace.m_mins.y + m_screenSpace.m_maxs.y * 0.5f), m_screenSpace.m_maxs);
        AddVertsForAABB2D(verts, playerController1.m_mins, playerController1.m_maxs, Rgba8::BLACK);
        AddVertsForDisc2D(verts, playerController1.GetCenter(), 150.f, 10.f, Rgba8::GREEN);
        AABB2 const playerController2 = AABB2(m_screenSpace.m_mins, Vec2(m_screenSpace.m_maxs.x, m_screenSpace.m_maxs.y * 0.5f));
        AddVertsForAABB2D(verts, playerController2.m_mins, playerController2.m_maxs, Rgba8::GREY);
        AddVertsForDisc2D(verts, playerController2.GetCenter(), 150.f, 10.f, Rgba8::RED);

        g_theRenderer->SetModelConstants();
        g_theRenderer->SetBlendMode(eBlendMode::ALPHA);
        g_theRenderer->SetRasterizerMode(eRasterizerMode::SOLID_CULL_NONE);
        g_theRenderer->SetSamplerMode(eSamplerMode::POINT_CLAMP);
        g_theRenderer->SetDepthMode(eDepthMode::DISABLED);
        g_theRenderer->BindTexture(nullptr);
        g_theRenderer->DrawVertexArray(static_cast<int>(verts.size()), verts.data());

        VertexList_PCU  textVerts;
        float constexpr textHeight                  = 20.f;
        AABB2 const     playerControllerTextBounds1 = AABB2(playerController1.m_mins, Vec2(playerController1.m_maxs.x, playerController2.m_maxs.y + textHeight * 4.f));
        AABB2 const     playerControllerTextBounds2 = AABB2(playerController2.m_mins, Vec2(playerController2.m_maxs.x, textHeight * 4.f));
        String const    text1                       = m_localPlayerControllerList[0]->m_deviceType == eDeviceType::KEYBOARD_AND_MOUSE ? "Keyboard and Mouse\nPress SPACE to start game\nPress ESCAPE to leave game" : "Controller\nPress START to start game\nPress BACK to leave game";
        String const    text2                       = m_localPlayerControllerList[1]->m_deviceType == eDeviceType::KEYBOARD_AND_MOUSE ? "Keyboard and Mouse\nPress SPACE to start game\nPress ESCAPE to leave game" : "Controller\nPress START to start game\nPress BACK to leave game";
        g_theBitmapFont->AddVertsForTextInBox2D(textVerts, "Player 01\n" + text1, playerControllerTextBounds1, textHeight * 4.f, Rgba8::WHITE, 1.f, Vec2::HALF);
        g_theBitmapFont->AddVertsForTextInBox2D(textVerts, "Player 02\n" + text2, playerControllerTextBounds2, textHeight * 4.f, Rgba8::WHITE, 1.f, Vec2::HALF);
        g_theRenderer->SetModelConstants();
        g_theRenderer->SetBlendMode(eBlendMode::ALPHA);
        g_theRenderer->SetRasterizerMode(eRasterizerMode::SOLID_CULL_NONE);
        g_theRenderer->SetSamplerMode(eSamplerMode::POINT_CLAMP);
        g_theRenderer->SetDepthMode(eDepthMode::DISABLED);
        g_theRenderer->BindTexture(&g_theBitmapFont->GetTexture());
        g_theRenderer->DrawVertexArray(static_cast<int>(textVerts.size()), textVerts.data());
    }
}

//----------------------------------------------------------------------------------------------------
void Game::RenderInGame() const
{
    for (PlayerController const* controller : m_localPlayerControllerList)
    {
        if (controller != nullptr && controller->m_isCameraMode)
        {
            DebugAddScreenText(Stringf("(F1)Control Mode:Player Camera"), Vec2::ZERO, 20.f, Vec2::ZERO, 0.f);
        }
        else
        {
            DebugAddScreenText(Stringf("(F1)Control Mode:Actor"), Vec2::ZERO, 20.f, Vec2::ZERO, 0.f);
        }
    }
}

//----------------------------------------------------------------------------------------------------
void Game::RenderPlayerController() const
{
    for (PlayerController const* controller : m_localPlayerControllerList)
    {
        if (controller != nullptr)
        {
            controller->Render();
        }
    }
}

//----------------------------------------------------------------------------------------------------
PlayerController* Game::CreateLocalPlayer(int const         id,
                                          eDeviceType const deviceType)
{
    PlayerController* newPlayer = new PlayerController(nullptr);
    newPlayer->SetInputDeviceType(deviceType);

    for (PlayerController const* controller : m_localPlayerControllerList)
    {
        if (controller && controller->GetControllerIndex() == id)
        {
            return nullptr;
        }
    }

    newPlayer->SetControllerIndex(id);
    m_localPlayerControllerList.push_back(newPlayer);

    return newPlayer;
}

//----------------------------------------------------------------------------------------------------
void Game::RemoveLocalPlayer(int id)
{
    std::vector<PlayerController*>::iterator const it = std::remove_if(m_localPlayerControllerList.begin(), m_localPlayerControllerList.end(),
                                                                       [id](PlayerController const* controller) {
                                                                           if (controller && controller->GetControllerIndex() == id)
                                                                           {
                                                                               printf("Game::RemoveLocalPlayer     Remove Local Player with id: %d\n", id);
                                                                               delete controller;
                                                                               return true;
                                                                           }
                                                                           return false;
                                                                       });
    // Another solution rather tha use remove_if
    // If you need to ensure that unused memory is freed, you can use the swap technique.
    m_localPlayerControllerList.erase(it, m_localPlayerControllerList.end());
    m_localPlayerControllerList.shrink_to_fit();
}

//----------------------------------------------------------------------------------------------------
PlayerController* Game::GetLocalPlayer(int const id) const
{
    for (PlayerController* m_localPlayerController : m_localPlayerControllerList)
    {
        if (m_localPlayerController &&
            m_localPlayerController->GetControllerIndex() == id)

            return m_localPlayerController;
    }

    return nullptr;
}

//----------------------------------------------------------------------------------------------------
PlayerController* Game::GetControllerByDeviceType(eDeviceType const deviceType) const
{
    for (PlayerController* m_localPlayerController : m_localPlayerControllerList)
    {
        if (m_localPlayerController &&
            m_localPlayerController->GetInputDeviceType() == deviceType)

            return m_localPlayerController;
    }

    return nullptr;
}

//----------------------------------------------------------------------------------------------------
bool Game::GetIsSingleMode() const
{
    return m_localPlayerControllerList.size() == 1;
}

//----------------------------------------------------------------------------------------------------
void Game::ChangeState(eGameState const nextState)
{
    if (m_currentGameState == nextState)
    {
        return;
    }

    m_currentGameState = nextState;
}

//---------------------------------------- ------------------------------------------------------------
eGameState Game::GetGameState() const
{
    return m_currentGameState;
}

//----------------------------------------------------------------------------------------------------
void Game::InitializeMaps()
{
    MapDefinition::InitializeMapDefs("Data/Definitions/MapDefinitions.xml");
    TileDefinition::InitializeTileDefs("Data/Definitions/TileDefinitions.xml");
    ActorDefinition::InitializeActorDefs("Data/Definitions/ProjectileActorDefinitions.xml");
    WeaponDefinition::InitializeWeaponDefs("Data/Definitions/WeaponDefinitions.xml");
    ActorDefinition::InitializeActorDefs("Data/Definitions/ActorDefinitions.xml");

    m_maps.reserve(1);

    for (int mapIndex = 0; mapIndex < 1; ++mapIndex)
    {
        m_maps.push_back(new Map(this, *MapDefinition::s_mapDefinitions[mapIndex]));
    }

    m_currentMap = m_maps[0];
}
