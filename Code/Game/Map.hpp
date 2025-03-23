//----------------------------------------------------------------------------------------------------
// Map.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/RaycastUtils.hpp"

//-Forward-Declaration--------------------------------------------------------------------------------
class Actor;
class IndexBuffer;
class VertexBuffer;
class Shader;
class Texture;
struct Tile;
struct MapDefinition;
class Game;

//----------------------------------------------------------------------------------------------------
class Map
{
public:
    Map(Game* owner, MapDefinition const& mapDef);
    ~Map();

    void CreateTiles();
    void CreateGeometry();
    void AddGeometryForWall(VertexList_PCUTBN& verts, std::vector<unsigned int>& indexes, AABB3 const& bounds, AABB2 const& UVs) const;
    void AddGeometryForFloor(VertexList_PCUTBN& verts, std::vector<unsigned int>& indexes, AABB3 const& bounds, AABB2 const& UVs) const;
    void AddGeometryForCeiling(VertexList_PCUTBN& verts, std::vector<unsigned int>& indexes, AABB3 const& bounds, AABB2 const& UVs) const;
    void CreateBuffers();

    bool  IsPositionInBounds(Vec3 position, float  tolerance = 0.f) const;
    bool  AreCoordsInBounds(int x, int y) const;
    Tile* GetTile(int x, int y) const;

    void Update();
    void CollideActors();
    void CollideActors(Actor* actorA, Actor* actorB);
    void CollideActorsWithMap();
    void CollideActorWithMap(Actor* actor);

    void Render() const;

    RaycastResult3D RaycastAll(Vec3 const& start, Vec3 const& direction, float distance) const;
    RaycastResult3D RaycastWorldXY(Vec3 const& start, Vec3 const& direction, float distance) const;
    RaycastResult3D RaycastWorldZ(Vec3 const& start, Vec3 const& direction, float distance) const;
    RaycastResult3D RaycastWorldActors(Vec3 const& start, Vec3 const& direction, float distance) const;

    Game* m_game = nullptr;

protected:
    // Map
    MapDefinition const* m_definition = nullptr;
    // std::vector<Tile>    m_tiles;
    IntVec2              m_dimensions;

    // Rendering
    // VertexList_PCUTBN         m_vertexes;
    std::vector<unsigned int> m_indexes;
    Texture const*            m_texture      = nullptr;
    Shader*                   m_shader       = nullptr;
    VertexBuffer*             m_vertexBuffer = nullptr;
    IndexBuffer*              m_indexBuffer  = nullptr;
};
