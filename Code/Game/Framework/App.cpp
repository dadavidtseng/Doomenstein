//----------------------------------------------------------------------------------------------------
// App.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Framework/App.hpp"

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Platform/Window.hpp"
#include "Game/Gameplay/Game.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Subsystem/Light/LightSubsystem.hpp"

//----------------------------------------------------------------------------------------------------
App*                   g_theApp            = nullptr;       // Created and owned by Main_Windows.cpp
AudioSystem*           g_theAudio          = nullptr;       // Created and owned by the App
BitmapFont*            g_theBitmapFont     = nullptr;       // Created and owned by the App
Game*                  g_theGame           = nullptr;       // Created and owned by the App
Renderer*              g_theRenderer       = nullptr;       // Created and owned by the App
RandomNumberGenerator* g_theRNG            = nullptr;       // Created and owned by the App
Window*                g_theWindow         = nullptr;       // Created and owned by the App
LightSubsystem*        g_theLightSubsystem = nullptr;       // Created and owned by the App

//----------------------------------------------------------------------------------------------------
STATIC bool App::m_isQuitting = false;

//----------------------------------------------------------------------------------------------------
void App::Startup()
{
    LoadGameConfig("Data/GameConfig.xml");

    // Create All Engine Subsystems
    sEventSystemConfig eventSystemConfig;
    g_theEventSystem = new EventSystem(eventSystemConfig);
    g_theEventSystem->SubscribeEventCallbackFunction("OnCloseButtonClicked", OnCloseButtonClicked);
    g_theEventSystem->SubscribeEventCallbackFunction("quit", OnCloseButtonClicked);

    sInputSystemConfig inputConfig;
    g_theInput = new InputSystem(inputConfig);

    sWindowConfig windowConfig;
    windowConfig.m_aspectRatio = 2.f;
    windowConfig.m_inputSystem = g_theInput;
    windowConfig.m_windowTitle = "Doomenstein";
    g_theWindow                = new Window(windowConfig);

    sRenderConfig renderConfig;
    renderConfig.m_window = g_theWindow;
    g_theRenderer         = new Renderer(renderConfig);

    sDebugRenderConfig debugConfig;
    debugConfig.m_renderer = g_theRenderer;
    debugConfig.m_fontName = "SquirrelFixedFont";

    // Initialize devConsoleCamera
    m_devConsoleCamera = new Camera();

    Vec2 const  bottomLeft     = Vec2::ZERO;
    float const screenSizeX    = g_gameConfigBlackboard.GetValue("screenSizeX", -1.f);
    float const screenSizeY    = g_gameConfigBlackboard.GetValue("screenSizeY", -1.f);
    Vec2 const  screenTopRight = Vec2(screenSizeX, screenSizeY);

    m_devConsoleCamera->SetOrthoGraphicView(bottomLeft, screenTopRight);

    sDevConsoleConfig devConsoleConfig;
    devConsoleConfig.m_defaultRenderer = g_theRenderer;
    devConsoleConfig.m_defaultFontName = "SquirrelFixedFont";
    devConsoleConfig.m_defaultCamera   = m_devConsoleCamera;
    g_theDevConsole                    = new DevConsole(devConsoleConfig);

    g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Controls");
    g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "(Mouse) Aim");
    g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "(W/A)   Move");
    g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "(S/D)   Strafe");
    g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "(Z/C)   Elevate");
    g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "(Shift) Sprint");
    g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "(1)     Pistol");
    g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "(2)     Plasma Rifle");
    g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "(P)     Pause");
    g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "(O)     Step Frame");
    g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "(F)     Toggle Free Camera");
    g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "(N)     Possess Next Actor");
    g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "(~)     Toggle Dev Console");
    g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "(ESC)   Exit Game");
    g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "(SPACE) Start Game");

    sAudioSystemConfig audioConfig;
    g_theAudio = new AudioSystem(audioConfig);

    sLightConfig constexpr lightConfig;
    g_theLightSubsystem = new LightSubsystem(lightConfig);

    g_theEventSystem->Startup();
    g_theWindow->Startup();
    g_theRenderer->Startup();
    DebugRenderSystemStartup(debugConfig);
    g_theDevConsole->StartUp();
    g_theInput->Startup();
    g_theAudio->Startup();
    g_theLightSubsystem->StartUp();

    g_theBitmapFont = g_theRenderer->CreateOrGetBitmapFontFromFile("Data/Fonts/SquirrelFixedFont"); // DO NOT SPECIFY FILE .EXTENSION!!  (Important later on.)
    g_theRNG        = new RandomNumberGenerator();
    g_theGame       = new Game();
}

//----------------------------------------------------------------------------------------------------
// All Destroy and ShutDown process should be reverse order of the StartUp
//
void App::Shutdown()
{
    // Destroy all Engine Subsystem
    delete g_theGame;
    g_theGame = nullptr;

    delete g_theRNG;
    g_theRNG = nullptr;

    delete g_theBitmapFont;
    g_theBitmapFont = nullptr;

    g_theLightSubsystem->ShutDown();
    g_theAudio->Shutdown();
    g_theInput->Shutdown();
    g_theDevConsole->Shutdown();

    delete m_devConsoleCamera;
    m_devConsoleCamera = nullptr;

    DebugRenderSystemShutdown();
    g_theRenderer->Shutdown();
    g_theWindow->Shutdown();
    g_theEventSystem->Shutdown();

    delete g_theAudio;
    g_theAudio = nullptr;

    delete g_theRenderer;
    g_theRenderer = nullptr;

    delete g_theWindow;
    g_theWindow = nullptr;

    delete g_theInput;
    g_theInput = nullptr;
}

//----------------------------------------------------------------------------------------------------
// One "frame" of the game.  Generally: Input, Update, Render.  We call this 60+ times per second.
//
void App::RunFrame()
{
    BeginFrame();   // Engine pre-frame stuff
    Update();       // Game updates / moves / spawns / hurts / kills stuff
    Render();       // Game draws current state of things
    EndFrame();     // Engine post-frame stuff
}

//----------------------------------------------------------------------------------------------------
void App::RunMainLoop()
{
    // Program main loop; keep running frames until it's time to quit
    while (!m_isQuitting)
    {
        // Sleep(16); // Temporary code to "slow down" our app to ~60Hz until we have proper frame timing in
        RunFrame();
    }
}

//----------------------------------------------------------------------------------------------------
STATIC bool App::OnCloseButtonClicked(EventArgs& args)
{
    UNUSED(args)

    RequestQuit();

    return true;
}

//----------------------------------------------------------------------------------------------------
STATIC void App::RequestQuit()
{
    m_isQuitting = true;
}

//----------------------------------------------------------------------------------------------------
void App::BeginFrame() const
{
    g_theEventSystem->BeginFrame();
    g_theWindow->BeginFrame();
    g_theRenderer->BeginFrame();
    DebugRenderBeginFrame();
    g_theDevConsole->BeginFrame();
    g_theInput->BeginFrame();
    g_theAudio->BeginFrame();
    g_theLightSubsystem->BeginFrame();
}

//----------------------------------------------------------------------------------------------------
void App::Update()
{
    Clock::TickSystemClock();

    UpdateCursorMode();

    g_theGame->Update();
}

//----------------------------------------------------------------------------------------------------
// Some simple OpenGL example drawing code.
// This is the graphical equivalent of printing "Hello, world."
//
// Ultimately this function (App::Render) will only call methods on Renderer (like Renderer::DrawVertexArray)
//	to draw things, never calling OpenGL (nor DirectX) functions directly.
//
void App::Render() const
{
    Rgba8 const clearColor = Rgba8::GREY;

    g_theRenderer->ClearScreen(clearColor);
    g_theGame->Render();

    g_theRenderer->BeginCamera(*m_devConsoleCamera);
    AABB2 const box = AABB2(Vec2::ZERO, Vec2(1600.f, 30.f));

    g_theDevConsole->Render(box);
    g_theRenderer->EndCamera(*m_devConsoleCamera);
}

//----------------------------------------------------------------------------------------------------
void App::EndFrame() const
{
    g_theEventSystem->EndFrame();
    g_theWindow->EndFrame();
    g_theRenderer->EndFrame();
    DebugRenderEndFrame();
    g_theDevConsole->EndFrame();
    g_theInput->EndFrame();
    g_theAudio->EndFrame();
    g_theLightSubsystem->EndFrame();
}

//----------------------------------------------------------------------------------------------------
void App::UpdateCursorMode()
{
    bool const doesWindowHasFocus   = GetActiveWindow() == g_theWindow->GetWindowHandle();
    bool const shouldUsePointerMode = !doesWindowHasFocus || g_theDevConsole->IsOpen() || g_theGame->GetGameState() == eGameState::ATTRACT;

    if (shouldUsePointerMode == true)
    {
        g_theInput->SetCursorMode(eCursorMode::POINTER);
    }
    else
    {
        g_theInput->SetCursorMode(eCursorMode::FPS);
    }
}

//----------------------------------------------------------------------------------------------------
void App::DeleteAndCreateNewGame()
{
    delete g_theGame;
    g_theGame = nullptr;

    g_theGame = new Game();
}

void App::LoadGameConfig(char const* gameConfigXmlFilePath)
{
    XmlDocument     gameConfigXml;
    XmlResult const result = gameConfigXml.LoadFile(gameConfigXmlFilePath);

    if (result == XmlResult::XML_SUCCESS)
    {
        if (XmlElement const* rootElement = gameConfigXml.RootElement())
        {
            g_gameConfigBlackboard.PopulateFromXmlElementAttributes(*rootElement);
        }
        else
        {
            DebuggerPrintf("WARNING: game config from file \"%s\" was invalid (missing root element)\n", gameConfigXmlFilePath);
        }
    }
    else
    {
        DebuggerPrintf("WARNING: failed to load game config from file \"%s\"\n", gameConfigXmlFilePath);
    }
}
