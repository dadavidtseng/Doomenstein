//----------------------------------------------------------------------------------------------------
// Player.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/EulerAngles.hpp"

//----------------------------------------------------------------------------------------------------
class Camera;
class Game;

//----------------------------------------------------------------------------------------------------
class Player
{
public:
    explicit Player(Game* owner);
    ~Player();

    void Update(float deltaSeconds);
    void Render() const;
    void UpdateFromKeyBoard();
    void UpdateFromController();

    Camera* GetCamera() const;
    Mat44 GetModelToWorldTransform() const;

    Game*       m_game            = nullptr;
    Vec3        m_position        = Vec3::ZERO;
    Vec3        m_velocity        = Vec3::ZERO;
    EulerAngles m_orientation     = EulerAngles::ZERO;
    EulerAngles m_angularVelocity = EulerAngles::ZERO;
    Rgba8       m_color           = Rgba8::WHITE;

private:
    Camera*     m_worldCamera     = nullptr;
};
