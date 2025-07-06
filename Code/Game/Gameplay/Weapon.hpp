//----------------------------------------------------------------------------------------------------
// Weapon.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once

#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Game/Framework/Animation.hpp"

//-Forward-Declaration--------------------------------------------------------------------------------
class Actor;
class Timer;
struct WeaponDefinition;

//----------------------------------------------------------------------------------------------------
class Weapon
{
public:
    explicit Weapon(Actor* owner, WeaponDefinition* weaponDef);
    ~Weapon();
    void Update(float deltaSeconds);
    void UpdateAnimation(float deltaSeconds);

    void        Render() const;
    void        RenderWeaponBase() const;
    void        RenderWeaponReticule() const;
    void        RenderWeaponHudText() const;
    void        RenderWeaponAnim() const;
    void        Fire();
    EulerAngles GetRandomDirectionInCone(EulerAngles weaponOrientation, float degreeOfVariation);
    Animation*  PlayAnimationByName(String animationName, bool force = false);

    Actor*            m_owner        = nullptr;
    WeaponDefinition* m_definition   = nullptr;     // Reference to our weapon definition.
    Timer*            m_timer        = nullptr;
    float             m_lastFireTime = 0.f;

    AABB2 m_hudBaseBound;   // we calculate the bound that Seamlessly connect the weapon texture

    Animation* m_currentPlayingAnimation = nullptr;
    Timer*     m_animationTimer          = nullptr;
};
