//----------------------------------------------------------------------------------------------------
// Actor.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Cylinder3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"

//----------------------------------------------------------------------------------------------------
class Actor
{
public:
    Actor(Vec3 const& position, EulerAngles const& orientation);

    void Update();
    void Render() const;
    Mat44 GetModelToWorldTransform() const;

    Vec3        m_position    = Vec3::ZERO;
    EulerAngles m_orientation = EulerAngles::ZERO;
    Rgba8       m_color       = Rgba8::WHITE;
    Cylinder3   m_cylinder;
};
