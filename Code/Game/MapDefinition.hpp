//----------------------------------------------------------------------------------------------------
// MapDefinition.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include "Engine/Core/Image.hpp"
#include "Engine/Core/XmlUtils.hpp"

//-Forward-Declaration--------------------------------------------------------------------------------
class Texture;
class Shader;

//----------------------------------------------------------------------------------------------------
struct SpawnInfo
{
    String m_name;
};

//----------------------------------------------------------------------------------------------------
struct MapDefinition
{
    MapDefinition() = default;
    ~MapDefinition();

    bool LoadFromXmlElement(XmlElement const& element);

    static void                        InitializeMapDefs(char const* path);
    static std::vector<MapDefinition*> s_mapDefinitions;

    String         m_name;
    Image          m_image              = Image(IntVec2::ZERO, Rgba8::WHITE);
    Shader*        m_shader             = nullptr;
    Texture const* m_spriteSheetTexture = nullptr;
    IntVec2        m_spriteSheetCellCount;
};
