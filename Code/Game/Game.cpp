//----------------------------------------------------------------------------------------------------
// Game.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Game.hpp"

#include "ActorDefinition.hpp"
#include "Map.hpp"
#include "MapDefinition.hpp"
#include "TileDefinition.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Game/PlayerController.hpp"

//----------------------------------------------------------------------------------------------------
Game::Game()
{
    SpawnPlayer();

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
    if (m_currentMap != nullptr)
    {
        delete m_currentMap;
        m_currentMap = nullptr;
    }

    if (m_gameClock != nullptr)
    {
        delete m_gameClock;
        m_gameClock = nullptr;
    }

    if (m_player != nullptr)
    {
        delete m_player;
        m_player = nullptr;
    }

    if (m_screenCamera != nullptr)
    {
        delete m_screenCamera;
        m_screenCamera = nullptr;
    }
}

//----------------------------------------------------------------------------------------------------
// All timers in the game, such as those required by weapons, should use the game clock.
void Game::Update()
{
    float const gameDeltaSeconds   = static_cast<float>(m_gameClock->GetDeltaSeconds());
    float const systemDeltaSeconds = static_cast<float>(Clock::GetSystemClock().GetDeltaSeconds());

    // #TODO: Select keyboard or controller

    UpdateEntities(gameDeltaSeconds, systemDeltaSeconds);
    UpdateFromKeyBoard();
    UpdateFromController();

    if (m_currentMap != nullptr &&
        !m_player->m_isMovable)
    {
        m_currentMap->Update();
    }
}

//----------------------------------------------------------------------------------------------------
void Game::Render() const
{
    //-Start-of-Game-Camera---------------------------------------------------------------------------

    if (m_player != nullptr)
    {
        g_theRenderer->BeginCamera(*m_player->GetCamera());

        if (m_currentGameState == eGameState::INGAME)
        {
            RenderEntities();

            if (m_currentMap != nullptr)
            {
                m_currentMap->Render();
            }
        }

        g_theRenderer->EndCamera(*m_player->GetCamera());
    }

    //-End-of-Game-Camera-----------------------------------------------------------------------------
    //------------------------------------------------------------------------------------------------

    if (m_currentGameState == eGameState::INGAME)
    {
        if (m_player != nullptr)
        {
            DebugRenderWorld(*m_player->GetCamera());
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

PlayerController* Game::GetPlayer() const
{
    return m_player;
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
            m_currentGameState = eGameState::INGAME;
            InitializeMaps();

            SpawnPlayer();
        }
    }

    if (m_currentGameState == eGameState::INGAME)
    {
        if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
        {
            m_currentGameState = eGameState::ATTRACT;

            if (m_player != nullptr)
            {
                delete m_player;
                m_player = nullptr;
            }

            if (m_currentMap != nullptr)
            {
                delete m_currentMap;
                m_currentMap = nullptr;
            }

            m_maps.clear();
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

        if (m_player != nullptr)
        {
            DebugAddMessage(Stringf("Player Position: (%.2f, %.2f, %.2f)", m_player->m_position.x, m_player->m_position.y, m_player->m_position.z), 0.f);
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
            InitializeMaps();

            SpawnPlayer();
        }
    }

    if (m_currentGameState == eGameState::INGAME)
    {
        if (controller.WasButtonJustPressed(XBOX_BUTTON_BACK))
        {
            m_currentGameState = eGameState::ATTRACT;

            if (m_player != nullptr)
            {
                delete m_player;
                m_player = nullptr;
            }

            if (m_currentMap != nullptr)
            {
                delete m_currentMap;
                m_currentMap = nullptr;
            }

            m_maps.clear();
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
void Game::UpdateEntities(float const gameDeltaSeconds, float const systemDeltaSeconds) const
{
    if (m_player != nullptr &&
        m_currentGameState == eGameState::INGAME)
    {
        m_player->Update(systemDeltaSeconds);
    }

    DebugAddScreenText(Stringf("Time: %.2f\nFPS: %.2f\nScale: %.1f", m_gameClock->GetTotalSeconds(), 1.f / gameDeltaSeconds, m_gameClock->GetTimeScale()), m_screenCamera->GetOrthographicTopRight() - Vec2(250.f, 60.f), 20.f, Vec2::ZERO, 0.f, Rgba8::WHITE, Rgba8::WHITE);
}

//----------------------------------------------------------------------------------------------------
void Game::RenderAttractMode() const
{
    DebugDrawRing(Vec2(800.f, 400.f), 300.f, 10.f, Rgba8::YELLOW);
}

//----------------------------------------------------------------------------------------------------
void Game::RenderInGame() const
{
    if (m_player->m_isMovable)
    {
        DebugAddScreenText(Stringf("(F1)Control Mode:Player Camera"), Vec2::ZERO, 20.f, Vec2::ZERO, 0.f);
    }
    else
    {
        DebugAddScreenText(Stringf("(F1)Control Mode:Actor"), Vec2::ZERO, 20.f, Vec2::ZERO, 0.f);
    }
}

//----------------------------------------------------------------------------------------------------
void Game::RenderEntities() const
{
    // g_theRenderer->SetModelConstants(m_player->GetModelToWorldTransform());
    m_player->Render();
}

//----------------------------------------------------------------------------------------------------
void Game::SpawnPlayer()
{
    m_player = new PlayerController(this);
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
    ActorDefinition::InitializeActorDefs("Data/Definitions/ActorDefinitions.xml");

    m_maps.reserve(1);

    for (int mapIndex = 0; mapIndex < 1; ++mapIndex)
    {
        m_maps.push_back(new Map(this, *MapDefinition::s_mapDefinitions[mapIndex]));
    }

    m_currentMap = m_maps[0];
}
