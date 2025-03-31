//----------------------------------------------------------------------------------------------------
// TileDefinition.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"

//----------------------------------------------------------------------------------------------------
struct TileDefinition
{
    TileDefinition() = default;
    ~TileDefinition();

    bool LoadFromXmlElement(XmlElement const& element);

    static void                         InitializeTileDefs(char const* path);
    static TileDefinition const*        GetDefByName(String const& name);
    static StringList                   GetTileNames();
    static std::vector<TileDefinition*> s_tileDefinitions;

    String  m_name;
    bool    m_isSolid;
    Rgba8   m_mapImagePixelColor;
    IntVec2 m_floorSpriteCoords;
    IntVec2 m_ceilingSpriteCoords;
    IntVec2 m_wallSpriteCoords;
};
