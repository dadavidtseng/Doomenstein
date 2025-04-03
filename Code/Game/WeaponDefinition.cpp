//----------------------------------------------------------------------------------------------------
// WeaponDefinition.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/WeaponDefinition.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

STATIC std::vector<WeaponDefinition*> WeaponDefinition::s_weaponDefinitions;

WeaponDefinition::~WeaponDefinition()
{
    for (WeaponDefinition const* weaponDef : s_weaponDefinitions)
    {
        delete weaponDef;
    }

    s_weaponDefinitions.clear();
}

bool WeaponDefinition::LoadFromXmlElement(XmlElement const* element)
{
    m_name = ParseXmlAttribute(*element, "name", "Unnamed");

    return true;
}

//----------------------------------------------------------------------------------------------------
void WeaponDefinition::InitializeWeaponDefs(char const* path)
{
    XmlDocument     document;
    XmlResult const result = document.LoadFile(path);

    if (result != XmlResult::XML_SUCCESS)
    {
        ERROR_AND_DIE("Failed to load XML file: %s\n", path)
    }

    XmlElement const* rootElement = document.RootElement();

    if (rootElement == nullptr)
    {
        ERROR_AND_DIE("XML file %s is missing a root element.\n", path)
    }

    XmlElement const* weaponDefinitionElement = rootElement->FirstChildElement();

    while (weaponDefinitionElement != nullptr)
    {
        String            elementName      = weaponDefinitionElement->Name();
        WeaponDefinition* weaponDefinition = new WeaponDefinition();

        if (weaponDefinition->LoadFromXmlElement(weaponDefinitionElement))
        {
            s_weaponDefinitions.push_back(weaponDefinition);
        }
        else
        {
            delete weaponDefinition;
            ERROR_AND_DIE("Failed to load actor definition: %s\n", elementName.c_str())
        }

        weaponDefinitionElement = weaponDefinitionElement->NextSiblingElement();
    }
}

WeaponDefinition const* WeaponDefinition::GetDefByName(String const& name)
{
    for (WeaponDefinition const* tileDef : s_weaponDefinitions)
    {
        if (tileDef->m_name == name)
        {
            return tileDef;
        }
    }

    return nullptr;
}
