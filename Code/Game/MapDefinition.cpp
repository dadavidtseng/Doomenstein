//----------------------------------------------------------------------------------------------------
// MapDefinition.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/MapDefinition.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Game/GameCommon.hpp"

//----------------------------------------------------------------------------------------------------
std::vector<MapDefinition*> MapDefinition::s_mapDefinitions;

//----------------------------------------------------------------------------------------------------
MapDefinition::MapDefinition(XmlElement const& mapDefElement)
{
    m_name                                  = ParseXmlAttribute(mapDefElement, "name", "Unnamed");
    String const imageFilePath              = ParseXmlAttribute(mapDefElement, "image", "Unnamed");
    String const shaderFilePath             = ParseXmlAttribute(mapDefElement, "shader", "Unnamed");
    String const spriteSheetTextureFilePath = ParseXmlAttribute(mapDefElement, "spriteSheetTexture", "Unnamed");
    m_spriteSheetCellCount                  = ParseXmlAttribute(mapDefElement, "spriteSheetCellCount", IntVec2::ZERO);

    m_image              = g_theRenderer->CreateImageFromFile(imageFilePath.c_str());
    m_shader             = g_theRenderer->CreateOrGetShaderFromFile(shaderFilePath.c_str(), eVertexType::VERTEX_PCUTBN);
    m_spriteSheetTexture = g_theRenderer->CreateOrGetTextureFromFile(spriteSheetTextureFilePath.c_str());
}

//----------------------------------------------------------------------------------------------------
MapDefinition::~MapDefinition()
{
    for (MapDefinition* mapDef: s_mapDefinitions)
    {
        delete mapDef;
    }

    s_mapDefinitions.clear();
}

//----------------------------------------------------------------------------------------------------
STATIC void MapDefinition::InitializeMapDefs()
{
    XmlDocument mapDefXml;

    if (mapDefXml.LoadFile("Data/Definitions/MapDefinitions.xml") != XmlResult::XML_SUCCESS)
    {
        return;
    }

    if (XmlElement* root = mapDefXml.FirstChildElement("MapDefinitions"))
    {
        for (XmlElement* element = root->FirstChildElement("MapDefinition"); element != nullptr; element = element->NextSiblingElement("MapDefinition"))
        {
            MapDefinition* mapDef = new MapDefinition(*element);
            s_mapDefinitions.push_back(mapDef);
        }
    }
}
