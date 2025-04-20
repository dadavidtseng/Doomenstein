//----------------------------------------------------------------------------------------------------
// ActorDefinition.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once

#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

class AnimationGroup;
enum class eBillboardType : int8_t;

//----------------------------------------------------------------------------------------------------
struct ActorDefinition
{
public:
    ActorDefinition() = default;
    ~ActorDefinition();

    bool LoadFromXmlElement(XmlElement const* element);

    static void             InitializeActorDefs(char const* path);
    static ActorDefinition* GetDefByName(String const& name);
    AnimationGroup*         GetAnimationGroupByName(String const& name);
    // Sound*          GetSoundByName(std::string& name);

    static std::vector<ActorDefinition*> s_actorDefinitions;

    String m_name;
    String m_faction        = "NEUTRAL";    // Faction for the actor. Determines which actors are enemies for purposes of AI target selection.
    int    m_health         = 0;            // Starting health for the actor.
    bool   m_canBePossessed = false;        // Determines whether this actor can be possessed, by either the player or an AI.
    float  m_corpseLifetime = 0.f;          // Time that the actor should linger after death, in seconds. For purposes of playing death animations and effects.
    bool   m_isVisible      = false;        // Specifies whether the actor should render.
    bool   m_dieOnSpawn     = false;

    // Collision
    float      m_radius             = 0.f;
    float      m_height             = 0.f;
    float      m_impulseOnCollide   = 0.f;
    FloatRange m_damageOnCollide    = FloatRange::ZERO;
    bool       m_collidesWithWorld  = false;
    bool       m_collidesWithActors = false;
    bool       m_dieOnCollide       = false;

    // Physics
    bool  m_simulated = false;
    bool  m_flying    = false;
    float m_walkSpeed = 0.f;
    float m_runSpeed  = 0.f;
    float m_turnSpeed = 0.f;
    float m_drag      = 0.f;

    // Camera
    float m_eyeHeight = 0.f;
    float m_cameraFOV = 0.f;

    // AI
    bool  m_aiEnabled   = false;
    float m_sightRadius = 0.f;
    float m_sightAngle  = 0.f;

    // Visuals
    Vec2                        m_size          = Vec2::ZERO;
    Vec2                        m_pivot         = Vec2::ZERO;
    eBillboardType              m_billboardType = eBillboardType::NONE;
    bool                        m_renderLit     = false;
    bool                        m_renderRounded = false;
    IntVec2                     m_cellCount     = IntVec2::ZERO;
    Shader*                     m_shader        = nullptr;
    SpriteSheet*                m_spriteSheet   = nullptr;
    std::vector<AnimationGroup> m_animationGroup;

    // Weapons
    StringList m_inventory;
};
