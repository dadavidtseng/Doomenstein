//----------------------------------------------------------------------------------------------------
// TileDefinition.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/TileDefinition.hpp"

#include "Engine/Core/EngineCommon.hpp"

//----------------------------------------------------------------------------------------------------
std::vector<TileDefinition*> TileDefinition::s_tileDefinitions;

//----------------------------------------------------------------------------------------------------
TileDefinition::TileDefinition(XmlElement const& tileDefElement)
{
    m_name                = ParseXmlAttribute(tileDefElement, "name", "Unnamed");
    m_isSolid             = ParseXmlAttribute(tileDefElement, "isSolid", false);
    m_mapImagePixelColor  = ParseXmlAttribute(tileDefElement, "mapImagePixelColor", Rgba8::WHITE);
    m_floorSpriteCoords   = ParseXmlAttribute(tileDefElement, "floorSpriteCoords", IntVec2::ZERO);
    m_ceilingSpriteCoords = ParseXmlAttribute(tileDefElement, "ceilingSpriteCoords", IntVec2::ZERO);
    m_wallSpriteCoords    = ParseXmlAttribute(tileDefElement, "wallSpriteCoords", IntVec2::ZERO);
}

//----------------------------------------------------------------------------------------------------
TileDefinition::~TileDefinition()
{
}

//----------------------------------------------------------------------------------------------------
STATIC void TileDefinition::InitializeTileDefs()
{
    XmlDocument mapDefXml;

    if (mapDefXml.LoadFile("Data/Definitions/TileDefinitions.xml") != XmlResult::XML_SUCCESS)
    {
        return;
    }

    if (XmlElement* root = mapDefXml.FirstChildElement("TileDefinitions"))
    {
        for (XmlElement* element = root->FirstChildElement("TileDefinition"); element != nullptr; element = element->NextSiblingElement("TileDefinition"))
        {
            TileDefinition* mapDef = new TileDefinition(*element);

            s_tileDefinitions.push_back(mapDef);
        }
    }
}
