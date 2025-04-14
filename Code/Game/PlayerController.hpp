//----------------------------------------------------------------------------------------------------
// PlayerController.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once

#include "Engine/Math/EulerAngles.hpp"
#include "Game/Controller.hpp"

//----------------------------------------------------------------------------------------------------
class Camera;
class Game;

//----------------------------------------------------------------------------------------------------
// Player controllers should be constructed by the game and told to possess their actor whenever it is spawned or respawned. Player controller can possess other possessable actors in the game.
class PlayerController final : public Controller
{
public:
    // Construction / Destruction
    explicit PlayerController(Map* owner);
    ~PlayerController() override;

    void Update(float deltaSeconds) override;
    void UpdateFromInput();
    void UpdateWorldCamera();
    void Render() const;

    Mat44   GetModelToWorldTransform() const;

    Vec3        m_position     = Vec3::ZERO;
    Vec3        m_velocity     = Vec3::ZERO;
    EulerAngles m_orientation  = EulerAngles::ZERO;
    bool        m_isCameraMode = false;
    Camera* m_worldCamera = nullptr;
};
