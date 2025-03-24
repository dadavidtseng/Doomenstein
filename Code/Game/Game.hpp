//----------------------------------------------------------------------------------------------------
// Game.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include <cstdint>
#include <vector>

class Map;
//----------------------------------------------------------------------------------------------------
class Camera;
class Clock;
class Player;

//----------------------------------------------------------------------------------------------------
enum class eGameState : int8_t
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
    Clock*            m_gameClock    = nullptr;

private:
    void UpdateFromKeyBoard();
    void UpdateFromController();
    void UpdateEntities(float gameDeltaSeconds, float systemDeltaSeconds) const;
    void RenderAttractMode() const;
    void RenderEntities() const;

    void SpawnPlayer();

    void InitializeMaps();

    Camera*           m_screenCamera = nullptr;
    Player*           m_player       = nullptr;
    eGameState        m_gameState    = eGameState::Attract;
    std::vector<Map*> m_maps;
    Map*              m_currentMap = nullptr;
};
