//----------------------------------------------------------------------------------------------------
// Map.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Map.hpp"

#include "GameCommon.hpp"
#include "MapDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"

//----------------------------------------------------------------------------------------------------
Map::Map(Game*                owner,
         MapDefinition const& mapDef)
    : m_game(owner),
      m_definition(&mapDef)
{
}

//----------------------------------------------------------------------------------------------------
Map::~Map()
{
}

//----------------------------------------------------------------------------------------------------
void Map::CreateTiles()
{
}

//----------------------------------------------------------------------------------------------------
void Map::CreateGeometry()
{
}

//----------------------------------------------------------------------------------------------------
void Map::AddGeometryForWall(VertexList_PCUTBN& verts, std::vector<unsigned int>& indexes, AABB3 const& bounds, AABB2 const& UVs) const
{
}

//----------------------------------------------------------------------------------------------------
void Map::AddGeometryForFloor(VertexList_PCUTBN& verts, std::vector<unsigned int>& indexes, AABB3 const& bounds, AABB2 const& UVs) const
{
}

//----------------------------------------------------------------------------------------------------
void Map::AddGeometryForCeiling(VertexList_PCUTBN& verts, std::vector<unsigned int>& indexes, AABB3 const& bounds, AABB2 const& UVs) const
{
}

//----------------------------------------------------------------------------------------------------
void Map::CreateBuffers()
{
}

//----------------------------------------------------------------------------------------------------
bool Map::IsPositionInBounds(Vec3 position, float const tolerance) const
{
    return false;
}

//----------------------------------------------------------------------------------------------------
bool Map::AreCoordsInBounds(int x, int y) const
{
    return false;
}

//----------------------------------------------------------------------------------------------------
Tile* Map::GetTile(int x, int y) const
{
    return nullptr;
}

//----------------------------------------------------------------------------------------------------
void Map::Update()
{
}

//----------------------------------------------------------------------------------------------------
void Map::CollideActors()
{
}

//----------------------------------------------------------------------------------------------------
void Map::CollideActors(Actor* actorA, Actor* actorB)
{
}

//----------------------------------------------------------------------------------------------------
void Map::CollideActorsWithMap()
{
}

//----------------------------------------------------------------------------------------------------
void Map::CollideActorWithMap(Actor* actor)
{
}

//----------------------------------------------------------------------------------------------------
void Map::Render() const
{
}

//----------------------------------------------------------------------------------------------------
RaycastResult3D Map::RaycastAll(Vec3 const& start, Vec3 const& direction, float distance) const
{
    return {};
}

//----------------------------------------------------------------------------------------------------
RaycastResult3D Map::RaycastWorldXY(Vec3 const& start,
                                    Vec3 const& direction,
                                    float       distance) const
{
    return {};
}

//----------------------------------------------------------------------------------------------------
RaycastResult3D Map::RaycastWorldZ(Vec3 const& start,
                                   Vec3 const& direction,
                                   float       distance) const
{
    return {};
}

//----------------------------------------------------------------------------------------------------
RaycastResult3D Map::RaycastWorldActors(Vec3 const& start,
                                        Vec3 const& direction,
                                        float       distance) const
{
    return {};
}
