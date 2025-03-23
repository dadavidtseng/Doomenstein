//----------------------------------------------------------------------------------------------------
// MapDefinition.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include "Engine/Core/Image.hpp"
#include "Engine/Core/XmlUtils.hpp"

class Texture;
class Shader;

struct MapDefinition
{
    explicit MapDefinition(XmlElement const& mapDefElement);
    ~MapDefinition();

    static void                        InitializeMapDefs();
    static std::vector<MapDefinition*> s_mapDefinitions;

    String   m_name;
    Image    m_image = Image(IntVec2::ZERO, Rgba8::WHITE);
    Shader*  m_shader             = nullptr;
    Texture* m_spriteSheetTexture = nullptr;
    IntVec2  m_spriteSheetCellCount;
};
