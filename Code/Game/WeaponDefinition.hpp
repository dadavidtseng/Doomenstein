//----------------------------------------------------------------------------------------------------
// WeaponDefinition.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/XmlUtils.hpp"

//----------------------------------------------------------------------------------------------------
struct WeaponDefinition
{
    explicit WeaponDefinition(XmlElement const& mapDefElement);
    ~WeaponDefinition();

    static void                          InitializeActorDefs();
    static std::vector<WeaponDefinition*> s_actorDefinitions;

    String m_name;
};
