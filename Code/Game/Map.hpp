//----------------------------------------------------------------------------------------------------
// Map.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"

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
    void AddGeometryForWall(VertexList_PCUTBN& verts, IndexList& indexes, AABB3 const& bounds, AABB2 const& UVs) const;
    void AddGeometryForFloor(VertexList_PCUTBN& verts, IndexList& indexes, AABB3 const& bounds, AABB2 const& UVs) const;
    void AddGeometryForCeiling(VertexList_PCUTBN& verts, IndexList& indexes, AABB3 const& bounds, AABB2 const& UVs) const;
    void CreateBuffers();

    bool        IsPositionInBounds(Vec3 position, float tolerance = 0.f) const;
    bool        IsTileCoordsOutOfBounds(int x, int y) const;
    Tile const* GetTile(int x, int y) const;

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
    std::vector<Tile>    m_tiles;
    IntVec2              m_dimensions;

    // Rendering
    VertexList_PCUTBN m_vertexes;
    IndexList         m_indexes;
    Texture const*    m_texture      = nullptr;
    Shader*           m_shader       = nullptr;
    VertexBuffer*     m_vertexBuffer = nullptr;
    IndexBuffer*      m_indexBuffer  = nullptr;

    ConstantBuffer* m_lightCBO         = nullptr;
    Vec3            m_sunDirection     = Vec3(2.f, 1.f, -1.f).GetNormalized();
    float           m_sunIntensity     = 0.85f;
    float           m_ambientIntensity = 0.35f;
};
