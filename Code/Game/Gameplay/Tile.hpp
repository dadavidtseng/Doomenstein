//----------------------------------------------------------------------------------------------------
// Tile.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/AABB3.hpp"

//----------------------------------------------------------------------------------------------------
struct Tile
{
    // IntVec2 m_coords = IntVec2::NEGATIVE_ONE;
    AABB3  m_bounds  = AABB3::NEG_ONE;   // Tile bounds are the world space bounds of the tile.
    String m_name    = "Unnamed";
    bool   m_isSolid = false;
};
