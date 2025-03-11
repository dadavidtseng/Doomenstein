//----------------------------------------------------------------------------------------------------
// Game.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once

//----------------------------------------------------------------------------------------------------
class Camera;
class Clock;
class Player;

//----------------------------------------------------------------------------------------------------
enum class eGameState
{
    Attract,
    Game
};

//----------------------------------------------------------------------------------------------------
class Game
{
public:
    Game();
    ~Game();

    void Update();
    void Render() const;
    bool IsAttractMode() const;

private:
    void UpdateFromKeyBoard();
    void UpdateFromController();
    void UpdateEntities(float gameDeltaSeconds, float systemDeltaSeconds) const;
    void RenderAttractMode() const;
    void RenderEntities() const;

    void SpawnPlayer();

    Camera*    m_screenCamera = nullptr;
    Player*    m_player       = nullptr;
    Clock*     m_gameClock    = nullptr;
    eGameState m_gameState    = eGameState::Attract;
};
