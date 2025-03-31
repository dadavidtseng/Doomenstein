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
MapDefinition::~MapDefinition()
{
    for (MapDefinition const* mapDef : s_mapDefinitions)
    {
        delete mapDef;
    }

    s_mapDefinitions.clear();
}

//----------------------------------------------------------------------------------------------------
bool MapDefinition::LoadFromXmlElement(XmlElement const& element)
{
    m_name                                  = ParseXmlAttribute(element, "name", "Unnamed");
    String const imageFilePath              = ParseXmlAttribute(element, "image", "Unnamed");
    String const shaderFilePath             = ParseXmlAttribute(element, "shader", "Unnamed");
    String const spriteSheetTextureFilePath = ParseXmlAttribute(element, "spriteSheetTexture", "Unnamed");
    m_spriteSheetCellCount                  = ParseXmlAttribute(element, "spriteSheetCellCount", IntVec2::ZERO);

    m_image              = g_theRenderer->CreateImageFromFile(imageFilePath.c_str());
    m_shader             = g_theRenderer->CreateOrGetShaderFromFile(shaderFilePath.c_str(), eVertexType::VERTEX_PCUTBN);
    m_spriteSheetTexture = g_theRenderer->CreateOrGetTextureFromFile(spriteSheetTextureFilePath.c_str());

    return true;
}

//----------------------------------------------------------------------------------------------------
STATIC void MapDefinition::InitializeMapDefs(char const* path)
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

    XmlElement const* mapDefinitionElement = rootElement->FirstChildElement();

    while (mapDefinitionElement != nullptr)
    {
        String         elementName   = mapDefinitionElement->Name();
        MapDefinition* mapDefinition = new MapDefinition();

        if (mapDefinition->LoadFromXmlElement(*mapDefinitionElement))
        {
            s_mapDefinitions.push_back(mapDefinition);
        }
        else
        {
            delete mapDefinition;
            ERROR_AND_DIE("Failed to load actor definition: %s\n", elementName.c_str())
        }

        mapDefinitionElement = mapDefinitionElement->NextSiblingElement();
    }

    // XmlDocument mapDefXml;
    //
    // if (mapDefXml.LoadFile("Data/Definitions/MapDefinitions.xml") != XmlResult::XML_SUCCESS)
    // {
    //     return;
    // }
    //
    // if (XmlElement* root = mapDefXml.FirstChildElement("MapDefinitions"))
    // {
    //     for (XmlElement* element = root->FirstChildElement("MapDefinition"); element != nullptr; element = element->NextSiblingElement("MapDefinition"))
    //     {
    //         MapDefinition* mapDef = new MapDefinition(*element);
    //         s_mapDefinitions.push_back(mapDef);
    //     }
    // }
}
