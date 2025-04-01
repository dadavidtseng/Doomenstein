//----------------------------------------------------------------------------------------------------
// Game.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include <cstdint>
#include <vector>

//----------------------------------------------------------------------------------------------------
class Map;
class Camera;
class Clock;
class PlayerController;

//----------------------------------------------------------------------------------------------------
enum class eGameState : int8_t
{
    NONE,
    ATTRACT,
    LOBBY,
    INGAME
};

//----------------------------------------------------------------------------------------------------
class Game
{
public:
    Game();
    ~Game();

    void              Update();
    void              Render() const;
    bool              IsAttractMode() const;
    Map*              GetCurrentMap() const;
    PlayerController* GetPlayerController() const;
    void              SpawnPlayerController();
    Clock* m_gameClock = nullptr;

private:
    void UpdateFromKeyBoard();
    void UpdateFromController();
    void UpdateEntities(float gameDeltaSeconds, float systemDeltaSeconds) const;
    void RenderAttractMode() const;
    void RenderInGame() const;
    void RenderEntities() const;

    void       InitializeMaps();
    void       ChangeState(eGameState nextState);
    eGameState GetGameState() const;

    Camera*           m_screenCamera     = nullptr;
    PlayerController* m_playerController = nullptr;
    eGameState        m_currentGameState = eGameState::ATTRACT;
    std::vector<Map*> m_maps;
    Map*              m_currentMap = nullptr;
};
