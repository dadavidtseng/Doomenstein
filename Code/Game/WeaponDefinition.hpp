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
    WeaponDefinition() = default;
    ~WeaponDefinition();

    bool LoadFromXmlElement(XmlElement const* element);

    static void                           InitializeWeaponDefs(char const* path);
    static WeaponDefinition const*        GetDefByName(String const& name);
    static std::vector<WeaponDefinition*> s_weaponDefinitions;

    String m_name;
};
