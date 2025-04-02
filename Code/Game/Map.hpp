//----------------------------------------------------------------------------------------------------
// Map.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once

#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/RaycastUtils.hpp"

class PlayerController;
struct ActorHandle;
struct SpawnInfo;
//-Forward-Declaration--------------------------------------------------------------------------------
class Actor;
class Game;
class IndexBuffer;
class Shader;
class Texture;
class VertexBuffer;
struct MapDefinition;
struct Tile;

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

    bool          IsPositionInBounds(Vec3 const& position, float tolerance = 0.f) const;
    bool          IsTileCoordsOutOfBounds(IntVec2 const& tileCoords) const;
    bool          IsTileCoordsOutOfBounds(int x, int y) const;
    bool          IsTileSolid(IntVec2 const& tileCoords) const;
    IntVec2 const GetTileCoordsFromWorldPos(Vec3 const& worldPosition) const;
    Tile const*   GetTile(int x, int y) const;
    Tile const*   GetTile(IntVec2 const& tileCoords) const;

    void Update(float deltaSeconds);
    void UpdateFromKeyboard();
    void UpdateAllActors(float deltaSeconds) const;

    void CollideActors();
    void CollideActors(Actor* actorA, Actor* actorB);
    void CollideActorsWithMap() const;
    void CollideActorWithMap(Actor* actor) const;

    void PushActorOutOfTileIfSolid(Actor* actor, IntVec2 const& tileCoords) const;
    void RenderAllActors() const;
    void RenderMap() const;

    void Render() const;

    RaycastResult3D RaycastAll(Vec3 const& startPosition, Vec3 const& forwardNormal, float maxLength) const;
    RaycastResult3D RaycastWorldXY(Vec3 const& startPosition, Vec3 const& forwardNormal, float maxLength) const;
    RaycastResult3D RaycastWorldZ(Vec3 const& startPosition, Vec3 const& forwardNormal, float maxLength) const;
    RaycastResult3D RaycastWorldActors(Vec3 const& startPosition, Vec3 const& forwardNormal, float maxLength) const;

    Actor*       SpawnActor(SpawnInfo const& spawnInfo);
    Actor*       GetActorByHandle(ActorHandle handle) const;
    Actor const* GetActorByName(String const& name);
    void         GetActorsByName(std::vector<Actor*>& actorList, String const& name);
    void         DeleteDestroyedActor();
    Actor*         SpawnPlayer(PlayerController* playerController);
    void         GetClosestVisibleEnemy();
    void         DebugPossessNext() const;

    Game* m_game = nullptr;

protected:
    // Map
    MapDefinition const* m_mapDefinition = nullptr;
    std::vector<Tile>    m_tiles;
    IntVec2              m_dimensions;

    // Rendering
    VertexList_PCUTBN m_vertexes;
    IndexList         m_indexes;
    Texture const*    m_texture      = nullptr;
    Shader*           m_shader       = nullptr;
    VertexBuffer*     m_vertexBuffer = nullptr;
    IndexBuffer*      m_indexBuffer  = nullptr;

    Vec3  m_sunDirection     = Vec3(2.f, 1.f, -1.f).GetNormalized();
    float m_sunIntensity     = 0.85f;
    float m_ambientIntensity = 0.35f;

    // Actor
    std::vector<Actor*>           m_actors;
    static constexpr unsigned int MAX_ACTOR_UID  = 0x0000fffeu;
    unsigned int                  m_nextActorUID = 0;
    PlayerController*             m_playerController = nullptr;
};
