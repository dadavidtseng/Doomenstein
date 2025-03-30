//----------------------------------------------------------------------------------------------------
// ActorDefinition.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once

#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/XmlUtils.hpp"


//----------------------------------------------------------------------------------------------------
struct ActorDefinition
{
    ActorDefinition() = default;
    ~ActorDefinition();

    bool LoadFromXmlElement(XmlElement const& element);

    static void                          InitializeActorDefs(char const* path);
    static std::vector<ActorDefinition*> s_actorDefinitions;

    String m_name;
    int    m_health = 0;
};
