//----------------------------------------------------------------------------------------------------
// PlayerController.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once

#include <cstdint>

#include "Engine/Math/EulerAngles.hpp"
#include "Game/Framework/Controller.hpp"

//----------------------------------------------------------------------------------------------------
class Camera;
class Game;

enum class eDeviceType : int8_t
{
    KEYBOARD_AND_MOUSE = 0,
    CONTROLLER = 1,
};

//----------------------------------------------------------------------------------------------------
// Player controllers should be constructed by the game and told to possess their actor whenever it is spawned or respawned. Player controller can possess other possessable actors in the game.
class PlayerController final : public Controller
{
public:
    // Construction / Destruction
    explicit PlayerController(Map* owner);
    ~PlayerController() override;

    void Update(float deltaSeconds) override;
    void UpdateInput(float deltaSeconds);
    void UpdateFromKeyboard(float  deltaSeconds);
    void UpdateFromController(float  deltaSeconds);
    void UpdateWorldCamera();
    void Render() const;

    /// Setter
    eDeviceType SetInputDeviceType(eDeviceType newDeviceType);
    eDeviceType GetInputDeviceType() const;
    Mat44       GetModelToWorldTransform() const;

    Vec3        m_position     = Vec3::ZERO;
    Vec3        m_velocity     = Vec3::ZERO;
    EulerAngles m_orientation  = EulerAngles::ZERO;
    bool        m_isCameraMode = false;
    // Camera*     m_worldCamera  = nullptr;
    eDeviceType m_deviceType   = eDeviceType::KEYBOARD_AND_MOUSE;
    float         m_speed = 0.f;
    float m_turnRate = 0.f;
};
