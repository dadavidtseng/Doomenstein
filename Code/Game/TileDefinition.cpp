//----------------------------------------------------------------------------------------------------
// TileDefinition.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/TileDefinition.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//----------------------------------------------------------------------------------------------------
std::vector<TileDefinition*> TileDefinition::s_tileDefinitions;

//----------------------------------------------------------------------------------------------------
TileDefinition::~TileDefinition()
{
    s_tileDefinitions.clear();
}

//----------------------------------------------------------------------------------------------------
bool TileDefinition::LoadFromXmlElement(XmlElement const& element)
{
    m_name                = ParseXmlAttribute(element, "name", "Unnamed");
    m_isSolid             = ParseXmlAttribute(element, "isSolid", false);
    m_mapImagePixelColor  = ParseXmlAttribute(element, "mapImagePixelColor", Rgba8::WHITE);
    m_floorSpriteCoords   = ParseXmlAttribute(element, "floorSpriteCoords", IntVec2::ZERO);
    m_ceilingSpriteCoords = ParseXmlAttribute(element, "ceilingSpriteCoords", IntVec2::ZERO);
    m_wallSpriteCoords    = ParseXmlAttribute(element, "wallSpriteCoords", IntVec2::ZERO);

    return true;
}

//----------------------------------------------------------------------------------------------------
STATIC void TileDefinition::InitializeTileDefs(char const* path)
{
    XmlDocument     document;
    XmlResult const result = document.LoadFile(path);

    if (result != XmlResult::XML_SUCCESS)
    {
        ERROR_AND_DIE("Failed to load XML file")
    }

    XmlElement const* rootElement = document.RootElement();

    if (rootElement == nullptr)
    {
        ERROR_AND_DIE("XML file %s is missing a root element.")
    }

    XmlElement const* tileDefinitionElement = rootElement->FirstChildElement();

    while (tileDefinitionElement != nullptr)
    {
        String          elementName    = tileDefinitionElement->Name();
        TileDefinition* tileDefinition = new TileDefinition();

        if (tileDefinition->LoadFromXmlElement(*tileDefinitionElement))
        {
            s_tileDefinitions.push_back(tileDefinition);
        }
        else
        {
            delete tileDefinition;
            ERROR_AND_DIE("Failed to load actor definition")
        }

        tileDefinitionElement = tileDefinitionElement->NextSiblingElement();
    }

    // XmlDocument mapDefXml;
    //
    // if (mapDefXml.LoadFile("Data/Definitions/TileDefinitions.xml") != XmlResult::XML_SUCCESS)
    // {
    //     return;
    // }
    //
    // if (XmlElement* root = mapDefXml.FirstChildElement("TileDefinitions"))
    // {
    //     for (XmlElement* element = root->FirstChildElement("TileDefinition"); element != nullptr; element = element->NextSiblingElement("TileDefinition"))
    //     {
    //         TileDefinition* mapDef = new TileDefinition(*element);
    //
    //         s_tileDefinitions.push_back(mapDef);
    //     }
    // }
}

//----------------------------------------------------------------------------------------------------
TileDefinition const* TileDefinition::GetDefByName(String const& name)
{
    for (TileDefinition const* tileDef : s_tileDefinitions)
    {
        if (tileDef->m_name == name)
        {
            return tileDef;
        }
    }

    return nullptr;
}

//----------------------------------------------------------------------------------------------------
StringList TileDefinition::GetTileNames()
{
    StringList tileNames;

    for (TileDefinition const* tileDef : s_tileDefinitions)
    {
        if (tileDef)
        {
            tileNames.push_back(tileDef->m_name);
        }
    }

    return tileNames;
}
