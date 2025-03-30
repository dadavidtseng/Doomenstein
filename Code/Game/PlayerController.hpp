//----------------------------------------------------------------------------------------------------
// PlayerController.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include <cstdint>

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Game/Controller.hpp"

//----------------------------------------------------------------------------------------------------
class Camera;
class Game;

//----------------------------------------------------------------------------------------------------
enum class eCameraMode : int8_t
{
    PLAYER,
    ACTOR
};

//----------------------------------------------------------------------------------------------------
class PlayerController : public Controller
{
public:
    // Construction / Destruction
    explicit PlayerController(Game* owner);
    ~PlayerController();
    PlayerController(PlayerController& copyFrom)             = delete;
    PlayerController(PlayerController&& moveFrom)            = delete;
    PlayerController& operator=(PlayerController& copyFrom)  = delete;
    PlayerController& operator=(PlayerController&& moveFrom) = delete;

    void Update(float deltaSeconds);
    void Render() const;
    void UpdateFromKeyBoard();
    void UpdateFromController();

    Camera* GetCamera() const;
    Mat44   GetModelToWorldTransform() const;


    Game*       m_game              = nullptr;
    Vec3        m_position          = Vec3::ZERO;
    Vec3        m_velocity          = Vec3::ZERO;
    EulerAngles m_orientation       = EulerAngles::ZERO;
    EulerAngles m_angularVelocity   = EulerAngles::ZERO;
    bool        m_isMovable         = true;
    Rgba8       m_color             = Rgba8::WHITE;
    eCameraMode m_cameraMode = eCameraMode::PLAYER;

private:
    Camera* m_worldCamera = nullptr;
};
