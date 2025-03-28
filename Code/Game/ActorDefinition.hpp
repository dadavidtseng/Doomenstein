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
    explicit ActorDefinition(XmlElement const& mapDefElement);
    ~ActorDefinition();

    static void                          InitializeActorDefs();
    static std::vector<ActorDefinition*> s_actorDefinitions;

    String         m_name;
};
