//----------------------------------------------------------------------------------------------------
// MapDefinition.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/MapDefinition.hpp"

#include "GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"

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
    DebuggerPrintf((imageFilePath + "\n").c_str());
    DebuggerPrintf((shaderFilePath + "\n").c_str());
    DebuggerPrintf((spriteSheetTextureFilePath + "\n").c_str());
    m_image              = g_theRenderer->CreateImageFromFile(imageFilePath.c_str());

    IntVec2 imageDimensions = m_image.GetDimensions();

    for (int i = 0; i < imageDimensions.x; ++i)
    {
        for (int j = 0; j < imageDimensions.y; ++j)
        {
            IntVec2 coords(i, j);
            DebuggerPrintf("(%d, %d, %d, %d)\n",m_image.GetTexelColor(coords).r,m_image.GetTexelColor(coords).g,m_image.GetTexelColor(coords).b,m_image.GetTexelColor(coords).a);
        }
    }

    m_shader             = g_theRenderer->CreateOrGetShaderFromFile(shaderFilePath.c_str(), eVertexType::VERTEX_PCUTBN);
    m_spriteSheetTexture = g_theRenderer->CreateOrGetTextureFromFile(spriteSheetTextureFilePath.c_str());

    if (XmlElement const* spawnInfosElement = mapDefElement.FirstChildElement("SpawnInfos"))
    {
        for (XmlElement const* spawnElement = spawnInfosElement->FirstChildElement("SpawnInfo");
             spawnElement != nullptr;
             spawnElement = spawnElement->NextSiblingElement("SpawnInfo"))
        {
            // m_spawnInfos.emplace_back(spawnElement);

            String actor = ParseXmlAttribute(*spawnElement, "actor", "Unnamed");
            String position = ParseXmlAttribute(*spawnElement, "position", "Unnamed");
            String orientation = ParseXmlAttribute(*spawnElement, "orientation", "Unnamed");
            DebuggerPrintf((actor + "\n").c_str());
            DebuggerPrintf((position + "\n").c_str());
            DebuggerPrintf((orientation + "\n").c_str());
        }
    }
}

MapDefinition::~MapDefinition()
{
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
