//----------------------------------------------------------------------------------------------------
// WeaponDefinition.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/FloatRange.hpp"

class HUD;
class Sound;

//----------------------------------------------------------------------------------------------------
struct WeaponDefinition
{
    WeaponDefinition() = default;
    ~WeaponDefinition();

    bool LoadFromXmlElement(XmlElement const* element);

    static void                           InitializeWeaponDefs(char const* path);
    static WeaponDefinition*              GetDefByName(String const& name);
    Sound*                                GetSoundByName(String const& soundName);
    static std::vector<WeaponDefinition*> s_weaponDefinitions;

    String     m_name;
    float      m_refireTime      = 0.f;                 // The time that must elapse between firing, in seconds.
    int        m_rayCount        = 0;                   // The number of rays to cast each time the weapon is fired. Each ray can potentially hit an actor and do damage.
    float      m_rayCone         = 0.f;                 // Maximum angle variation for each ray cast, in degrees. Each shot fired should be randomly distributed in a cone of this angle relative forward direction of the firing actor.
    float      m_rayRange        = 0.f;                 // The distance of each ray cast, in world units.
    FloatRange m_rayDamage       = FloatRange::ZERO;    // Minimum and maximum damage expressed as a float range. Each shot fired should do a random amount of damage in this range.
    float      m_rayImpulse      = 0.f;                 // The amount of impulse to impart to any actor hit by a ray. Impulse should be in the direction of the ray.
    int        m_projectileCount = 0;                   // Number of projectiles to launch each time the weapon is fired.
    float      m_projectileCone  = 0.f;                 // Maximum angle variation in degrees for each projectile launched cast. Each projectile launched should have its velocity randomly distributed in a cone of this angle relative to the forward direction of the firing actor.
    float      m_projectileSpeed = 0.f;                 // Magnitude of the velocity given to each projectile launched.
    String     m_projectileActor;                       // Definition name for the actor that should be spawned when a projectile is launched.
    int        m_meleeCount   = 0;                      // Number of melee attacks that should occur each time the weapon is fired.
    float      m_meleeArc     = 0.f;                    // Arc in which melee attacks occur, in degrees.
    float      m_meleeRange   = 0.f;                    // Range of each melee attack, in world units.
    FloatRange m_meleeDamage  = FloatRange::ZERO;       // Minimum and maximum damage expressed as a float range. Each melee attack should do a random amount of damage in this range.
    float      m_meleeImpulse = 0.f;                    // The amount of impulse to impart to any actor hit by a melee attack. Impulse should be in the forward direction of the firing actor.

    HUD*               m_hud = nullptr;
    std::vector<Sound> m_sounds;
};
