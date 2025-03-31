//----------------------------------------------------------------------------------------------------
// ActorDefinition.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once

#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/XmlUtils.hpp"


enum class eActorFaction : int8_t;

//----------------------------------------------------------------------------------------------------
struct ActorDefinition
{
    ActorDefinition() = default;
    ~ActorDefinition();

    bool LoadFromXmlElement(XmlElement const* element);

    static void                   InitializeActorDefs(char const* path);
    static ActorDefinition const* GetDefByName(String const& name);

    static std::vector<ActorDefinition*> s_actorDefinitions;

    String m_name;
    String m_faction        = "NEUTRAL";    // Faction for the actor. Determines which actors are enemies for purposes of AI target selection.
    int    m_health         = 0;            // Starting health for the actor.
    bool   m_canBePossessed = false;        // Determines whether this actor can be possessed, by either the player or an AI.
    float  m_corpseLifetime = 0.f;          // Time that the actor should linger after death, in seconds. For purposes of playing death animations and effects.
    bool   m_isVisible      = false;        // Specifies whether the actor should render.

    // Collision
    float m_radius             = 0.f;
    float m_height             = 0.f;
    bool  m_collidesWithWorld  = false;
    bool  m_collidesWithActors = false;

    // Physics
    bool  m_isSimulated = false;
    float m_walkSpeed   = 0.f;
    float m_runSpeed    = 0.f;
    float m_turnSpeed   = 0.f;
    float m_drag        = 0.f;

    // Camera
    float m_eyeHeight = 0.f;
    float m_cameraFOV = 0.f;

    // Visuals
};
