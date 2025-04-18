//----------------------------------------------------------------------------------------------------
// Game.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Game.hpp"

#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Input/InputSystem.hpp"
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
    // SpawnPlayer();
    // m_playerController = new PlayerController(m_currentMap);

    m_screenCamera = new Camera();

    Vec2 const bottomLeft     = Vec2::ZERO;
    Vec2 const screenTopRight = Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y);

    m_screenCamera->SetOrthoGraphicView(bottomLeft, screenTopRight);

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
    SafeDeletePointer(m_playerController);
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

    if (m_currentMap != nullptr &&
        m_playerController != nullptr)
    {
        m_currentMap->Update(gameDeltaSeconds);
    }
}

//----------------------------------------------------------------------------------------------------
void Game::Render() const
{
    //-Start-of-Game-Camera---------------------------------------------------------------------------

    if (m_playerController != nullptr)
    {
        g_theRenderer->BeginCamera(*m_playerController->m_worldCamera);

        if (m_currentGameState == eGameState::INGAME)
        {
            RenderPlayerController();

            if (m_currentMap != nullptr)
            {
                m_currentMap->Render();
            }
        }

        g_theRenderer->EndCamera(*m_playerController->m_worldCamera);
    }

    //-End-of-Game-Camera-----------------------------------------------------------------------------
    //------------------------------------------------------------------------------------------------

    if (m_currentGameState == eGameState::INGAME)
    {
        if (m_playerController != nullptr)
        {
            DebugRenderWorld(*m_playerController->m_worldCamera);
        }
    }

    //------------------------------------------------------------------------------------------------
    //-Start-of-Screen-Camera-------------------------------------------------------------------------

    g_theRenderer->BeginCamera(*m_screenCamera);

    if (m_currentGameState == eGameState::ATTRACT)
    {
        RenderAttractMode();
    }

    if (m_currentGameState == eGameState::INGAME)
    {
        RenderInGame();
    }

    g_theRenderer->EndCamera(*m_screenCamera);

    //-End-of-Screen-Camera---------------------------------------------------------------------------

    if (m_currentGameState == eGameState::INGAME)
    {
        DebugRenderScreen(*m_screenCamera);
    }
}

//----------------------------------------------------------------------------------------------------
bool Game::IsAttractMode() const
{
    return m_currentGameState == eGameState::ATTRACT;
}

//----------------------------------------------------------------------------------------------------
Map* Game::GetCurrentMap() const
{
    if (m_currentMap == nullptr)
    {
        ERROR_AND_DIE("(Game::GetCurrentMap) m_currentMap is nullptr")
    }

    return m_currentMap;
}

PlayerController* Game::GetPlayerController() const
{
    if (m_playerController == nullptr) return nullptr;

    return m_playerController;
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
            ChangeState(m_currentGameState = eGameState::INGAME);
            // SpawnPlayerController();
            InitializeMaps();
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

            if (m_playerController != nullptr)
            {
                delete m_playerController;
                m_playerController = nullptr;
            }
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

        if (m_playerController != nullptr)
        {
            DebugAddMessage(Stringf("PlayerController Position: (%.2f, %.2f, %.2f)", m_playerController->m_position.x, m_playerController->m_position.y, m_playerController->m_position.z), 0.f);
        }
    }
}

//----------------------------------------------------------------------------------------------------
void Game::UpdateFromController()
{
    XboxController const& controller = g_theInput->GetController(0);

    if (m_currentGameState == eGameState::ATTRACT)
    {
        if (controller.WasButtonJustPressed(XBOX_BUTTON_BACK))
        {
            App::RequestQuit();
        }

        if (controller.WasButtonJustPressed(XBOX_BUTTON_START))
        {
            m_currentGameState = eGameState::INGAME;
            // SpawnPlayerController();
            InitializeMaps();
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

            if (m_playerController != nullptr)
            {
                delete m_playerController;
                m_playerController = nullptr;
            }
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
    if (m_playerController != nullptr &&
        m_currentGameState == eGameState::INGAME)
    {
        float systemDeltaSeconds = (float)Clock::GetSystemClock().GetDeltaSeconds();
        m_playerController->Update(systemDeltaSeconds);
    }

    DebugAddScreenText(Stringf("Time: %.2f\nFPS: %.2f\nScale: %.1f", m_gameClock->GetTotalSeconds(), 1.f / deltaSeconds, m_gameClock->GetTimeScale()), m_screenCamera->GetOrthographicTopRight() - Vec2(250.f, 60.f), 20.f, Vec2::ZERO, 0.f, Rgba8::WHITE, Rgba8::WHITE);
}

//----------------------------------------------------------------------------------------------------
void Game::RenderAttractMode() const
{
    DebugDrawRing(Vec2(800.f, 400.f), 300.f, 10.f, Rgba8::YELLOW);
}

//----------------------------------------------------------------------------------------------------
void Game::RenderInGame() const
{
    if (m_playerController->m_isCameraMode)
    {
        DebugAddScreenText(Stringf("(F1)Control Mode:Player Camera"), Vec2::ZERO, 20.f, Vec2::ZERO, 0.f);
    }
    else
    {
        DebugAddScreenText(Stringf("(F1)Control Mode:Actor"), Vec2::ZERO, 20.f, Vec2::ZERO, 0.f);
    }
}

//----------------------------------------------------------------------------------------------------
void Game::RenderPlayerController() const
{
    if (m_playerController == nullptr) return;
    // g_theRenderer->SetModelConstants(m_player->GetModelToWorldTransform());
    m_playerController->Render();
}

//----------------------------------------------------------------------------------------------------
void Game::SpawnPlayerController()
{
    m_playerController = new PlayerController(m_currentMap);
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

//----------------------------------------------------------------------------------------------------
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
