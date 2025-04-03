//----------------------------------------------------------------------------------------------------
// Map.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Map.hpp"

#include "ActorDefinition.hpp"
#include "ActorHandle.hpp"
#include "Game.hpp"
#include "PlayerController.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Game/Actor.hpp"
#include "Game/GameCommon.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/Tile.hpp"
#include "Game/TileDefinition.hpp"

//----------------------------------------------------------------------------------------------------
Map::Map(Game*                owner,
         MapDefinition const& mapDef)
    : m_game(owner),
      m_mapDefinition(&mapDef)
{
    m_dimensions = m_mapDefinition->m_image.GetDimensions();

    m_vertexes.reserve(sizeof(AABB3) * m_dimensions.x * m_dimensions.y);
    m_tiles.reserve(static_cast<unsigned int>(m_dimensions.x * m_dimensions.y));
    m_actors.reserve(100);

    m_texture = m_mapDefinition->m_spriteSheetTexture;
    m_shader  = m_mapDefinition->m_shader;

    CreateBuffers();
    CreateTiles();
    CreateGeometry();

    // m_actors.push_back(new Actor(Vec3(7.5f, 8.5f, 0.25f), EulerAngles::ZERO, 0.35f, 0.75f, false, Rgba8::RED));
    // m_actors.push_back(new Actor(Vec3(8.5f, 8.5f, 0.125f), EulerAngles::ZERO, 0.35f, 0.75f, false, Rgba8::RED));
    // m_actors.push_back(new Actor(Vec3(9.5f, 8.5f, 0.f), EulerAngles::ZERO, 0.35f, 0.75f, false, Rgba8::RED));
    // m_actors.push_back(new Actor(Vec3(5.5f, 8.5f, 0.f), EulerAngles::ZERO, 0.0625f, 0.125f, true, Rgba8::BLUE));

    for (SpawnInfo const& spawnInfo : m_mapDefinition->m_spawnInfos)
    {
        SpawnActor(spawnInfo);
    }

    m_game->SpawnPlayerController();
    Actor* playerActor = SpawnPlayer(m_game->GetPlayerController());
    m_game->GetPlayerController()->Possess(playerActor->m_handle);

    // for (int actorIndex = 0; actorIndex < 5; ++actorIndex)
    // {
    //     m_actors.push_back(new Actor(*ActorDefinition::s_actorDefinitions[actorIndex]));
    // }
}

//----------------------------------------------------------------------------------------------------
Map::~Map()
{
    if (m_vertexBuffer != nullptr)
    {
        delete m_vertexBuffer;
        m_vertexBuffer = nullptr;
    }

    if (m_indexBuffer != nullptr)
    {
        delete m_indexBuffer;
        m_indexBuffer = nullptr;
    }

    m_vertexes.clear();
    m_tiles.clear();
    m_indexes.clear();
}

//----------------------------------------------------------------------------------------------------
void Map::CreateTiles()
{
    for (int i = 0; i < m_dimensions.x; ++i)
    {
        for (int j = 0; j < m_dimensions.y; ++j)
        {
            m_tiles.emplace_back();
        }
    }

    for (int i = 0; i < m_dimensions.x; ++i)
    {
        for (int j = 0; j < m_dimensions.y; ++j)
        {
            AABB3 const bounds = AABB3(Vec3(i, j, 0), Vec3(i + 1, j + 1, 1));
            IntVec2     coords = IntVec2(bounds.m_mins.x, bounds.m_mins.y);

            for (TileDefinition const* tileDef : TileDefinition::s_tileDefinitions)
            {
                if (m_mapDefinition->m_image.GetTexelColor(coords) == tileDef->m_mapImagePixelColor)
                {
                    m_tiles[j + i * m_dimensions.y].m_bounds  = bounds;
                    m_tiles[j + i * m_dimensions.y].m_name    = tileDef->m_name;
                    m_tiles[j + i * m_dimensions.y].m_isSolid = tileDef->m_isSolid;
                }
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------
void Map::CreateGeometry()
{
    unsigned int      indexOffset          = 0;
    IntVec2 const     spriteSheetCellCount = m_mapDefinition->m_spriteSheetCellCount;
    SpriteSheet const spriteSheet          = SpriteSheet(*m_mapDefinition->m_spriteSheetTexture, spriteSheetCellCount);

    for (int i = 0; i < m_dimensions.x; ++i)
    {
        for (int j = 0; j < m_dimensions.y; ++j)
        {
            AABB3 const bounds = AABB3(Vec3(i, j, 0), Vec3(i + 1, j + 1, 1));

            IntVec2 currentTileCoords = IntVec2(i, j);
            AABB2   wallUVs, floorUVs, ceilingUVs;

            for (TileDefinition const* tileDef : TileDefinition::s_tileDefinitions)
            {
                if (m_mapDefinition->m_image.GetTexelColor(currentTileCoords) == tileDef->m_mapImagePixelColor)
                {
                    wallUVs    = spriteSheet.GetSpriteUVs(tileDef->m_wallSpriteCoords.x + tileDef->m_wallSpriteCoords.y * 8);
                    floorUVs   = spriteSheet.GetSpriteUVs(tileDef->m_floorSpriteCoords.x + tileDef->m_floorSpriteCoords.y * 8);
                    ceilingUVs = spriteSheet.GetSpriteUVs(tileDef->m_ceilingSpriteCoords.x + tileDef->m_ceilingSpriteCoords.y * 8);
                }
            }

            AddGeometryForWall(m_vertexes, m_indexes, bounds, wallUVs);
            AddGeometryForFloor(m_vertexes, m_indexes, bounds, floorUVs);
            AddGeometryForCeiling(m_vertexes, m_indexes, bounds, ceilingUVs);
        }

        indexOffset += 4;
    }
}

//----------------------------------------------------------------------------------------------------
void Map::AddGeometryForWall(VertexList_PCUTBN& verts,
                             IndexList&         indexes,
                             AABB3 const&       bounds,
                             AABB2 const&       UVs) const
{
    Vec3 const frontBottomLeft  = Vec3(bounds.m_maxs.x, bounds.m_maxs.y - 1.f, bounds.m_maxs.z - 1.f);
    Vec3 const frontBottomRight = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z - 1.f);
    Vec3 const frontTopLeft     = Vec3(bounds.m_maxs.x, bounds.m_maxs.y - 1.f, bounds.m_maxs.z);
    Vec3 const frontTopRight    = bounds.m_maxs;
    Vec3 const backBottomLeft   = Vec3(bounds.m_mins.x, bounds.m_mins.y + 1.f, 0.f);
    Vec3 const backBottomRight  = bounds.m_mins;
    Vec3 const backTopLeft      = Vec3(bounds.m_mins.x, bounds.m_mins.y + 1.f, 1.f);
    Vec3 const backTopRight     = Vec3(bounds.m_mins.x, bounds.m_mins.y, 1.f);

    IntVec2 const currentTileCoords = IntVec2(bounds.m_mins.x, bounds.m_mins.y);
    Tile const*   currentTile       = GetTile(currentTileCoords.x, currentTileCoords.y);

    if (currentTile->m_name == "BrickWall")
    {
        AddVertsForQuad3D(verts, indexes, frontBottomLeft, frontBottomRight, frontTopLeft, frontTopRight, Rgba8::WHITE, UVs);        // Front
        AddVertsForQuad3D(verts, indexes, backBottomLeft, backBottomRight, backTopLeft, backTopRight, Rgba8::WHITE, UVs);            // Back
        AddVertsForQuad3D(verts, indexes, backBottomRight, frontBottomLeft, backTopRight, frontTopLeft, Rgba8::WHITE, UVs);          // Left
        AddVertsForQuad3D(verts, indexes, frontBottomRight, backBottomLeft, frontTopRight, backTopLeft, Rgba8::WHITE, UVs);          // Right
    }
}

//----------------------------------------------------------------------------------------------------
void Map::AddGeometryForFloor(VertexList_PCUTBN& verts,
                              IndexList&         indexes,
                              AABB3 const&       bounds,
                              AABB2 const&       UVs) const
{
    Vec3 const frontBottomLeft  = Vec3(bounds.m_maxs.x, bounds.m_maxs.y - 1.f, bounds.m_maxs.z - 1.f);
    Vec3 const frontBottomRight = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z - 1.f);
    Vec3 const backBottomLeft   = Vec3(bounds.m_mins.x, bounds.m_mins.y + 1.f, 0.f);
    Vec3 const backBottomRight  = bounds.m_mins;

    IntVec2 const currentTileCoords = IntVec2(bounds.m_mins.x, bounds.m_mins.y);
    Tile const*   currentTile       = GetTile(currentTileCoords.x, currentTileCoords.y);

    if (currentTile->m_name == "StoneFloor")
    {
        AddVertsForQuad3D(verts, indexes, backBottomLeft, backBottomRight, frontBottomRight, frontBottomLeft, Rgba8::WHITE, UVs);
    }
}

//----------------------------------------------------------------------------------------------------
void Map::AddGeometryForCeiling(VertexList_PCUTBN& verts,
                                IndexList&         indexes,
                                AABB3 const&       bounds,
                                AABB2 const&       UVs) const
{
    Vec3 const frontTopLeft  = Vec3(bounds.m_maxs.x, bounds.m_maxs.y - 1.f, bounds.m_maxs.z);
    Vec3 const frontTopRight = bounds.m_maxs;
    Vec3 const backTopLeft   = Vec3(bounds.m_mins.x, bounds.m_mins.y + 1.f, 1.f);
    Vec3 const backTopRight  = Vec3(bounds.m_mins.x, bounds.m_mins.y, 1.f);

    IntVec2 const currentTileCoords = IntVec2(bounds.m_mins.x, bounds.m_mins.y);
    Tile const*   currentTile       = GetTile(currentTileCoords.x, currentTileCoords.y);

    if (currentTile->m_name == "StoneFloor")
    {
        AddVertsForQuad3D(verts, indexes, backTopRight, backTopLeft, frontTopLeft, frontTopRight, Rgba8::WHITE, UVs);
    }
}

//----------------------------------------------------------------------------------------------------
void Map::CreateBuffers()
{
    m_vertexBuffer = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCUTBN), sizeof(Vertex_PCUTBN));
    m_indexBuffer  = g_theRenderer->CreateIndexBuffer(sizeof(Vertex_PCUTBN), sizeof(Vertex_PCUTBN));
}

//----------------------------------------------------------------------------------------------------
bool Map::IsPositionInBounds(Vec3 const& position,
                             float const tolerance) const
{
    return
        position.x > 0.f + tolerance &&
        position.y > 0.f - tolerance &&
        position.x < static_cast<float>(m_dimensions.x) + tolerance &&
        position.y < static_cast<float>(m_dimensions.y) + tolerance;
}

//----------------------------------------------------------------------------------------------------
bool Map::IsTileCoordsOutOfBounds(IntVec2 const& tileCoords) const
{
    return IsTileCoordsOutOfBounds(tileCoords.x, tileCoords.y);
}

//----------------------------------------------------------------------------------------------------
bool Map::IsTileCoordsOutOfBounds(int const x,
                                  int const y) const
{
    return
        x < 0 ||
        x >= m_dimensions.x ||
        y < 0 ||
        y >= m_dimensions.y;
}

//----------------------------------------------------------------------------------------------------
bool Map::IsTileSolid(IntVec2 const& tileCoords) const
{
    return GetTile(tileCoords.x, tileCoords.y)->m_isSolid;
}

//----------------------------------------------------------------------------------------------------
IntVec2 const Map::GetTileCoordsFromWorldPos(Vec3 const& worldPosition) const
{
    int const tileX = RoundDownToInt(worldPosition.x);
    int const tileY = RoundDownToInt(worldPosition.y);

    return IntVec2(tileX, tileY);
}

//----------------------------------------------------------------------------------------------------
Tile const* Map::GetTile(int const x,
                         int const y) const
{
    if (IsTileCoordsOutOfBounds(x, y))
    {
        ERROR_AND_DIE("tileCoords is out of bound!")
    }

    return &m_tiles[y + x * m_dimensions.y];
}

//----------------------------------------------------------------------------------------------------
Tile const* Map::GetTile(IntVec2 const& tileCoords) const
{
    return GetTile(tileCoords.x, tileCoords.y);
}

//----------------------------------------------------------------------------------------------------
void Map::Update(float const deltaSeconds)
{
    CollideActors();
    CollideActorsWithMap();
    UpdateFromKeyboard();
    UpdateAllActors(deltaSeconds);
}

//----------------------------------------------------------------------------------------------------
void Map::UpdateFromKeyboard()
{
    if (g_theInput->WasKeyJustPressed(KEYCODE_F2))
    {
        m_sunDirection.x -= 1.f;
        DebugAddMessage(Stringf("Sun Direction: (%.2f, %.2f, %.2f)", m_sunDirection.x, m_sunDirection.y, m_sunDirection.z), 5.f);
    }

    if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
    {
        m_sunDirection.x += 1.f;
        DebugAddMessage(Stringf("Sun Direction: (%.2f, %.2f, %.2f)", m_sunDirection.x, m_sunDirection.y, m_sunDirection.z), 5.f);
    }

    if (g_theInput->WasKeyJustPressed(KEYCODE_F4))
    {
        m_sunDirection.y -= 1.f;
        DebugAddMessage(Stringf("Sun Direction: (%.2f, %.2f, %.2f)", m_sunDirection.x, m_sunDirection.y, m_sunDirection.z), 5.f);
    }

    if (g_theInput->WasKeyJustPressed(KEYCODE_F5))
    {
        m_sunDirection.y += 1.f;
        DebugAddMessage(Stringf("Sun Direction: (%.2f, %.2f, %.2f)", m_sunDirection.x, m_sunDirection.y, m_sunDirection.z), 5.f);
    }

    if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
    {
        m_sunIntensity -= 0.05f;
        m_sunIntensity = GetClampedZeroToOne(m_sunIntensity);
        DebugAddMessage(Stringf("Sun Intensity: (%.2f)", m_sunIntensity), 5.f);
    }

    if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
    {
        m_sunIntensity += 0.05f;
        m_sunIntensity = GetClampedZeroToOne(m_sunIntensity);
        DebugAddMessage(Stringf("Sun Intensity: (%.2f)", m_sunIntensity), 5.f);
    }

    if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
    {
        m_ambientIntensity -= 0.05f;
        m_ambientIntensity = GetClampedZeroToOne(m_ambientIntensity);
        DebugAddMessage(Stringf("Ambient Intensity: (%.2f)", m_ambientIntensity), 5.f);
    }

    if (g_theInput->WasKeyJustPressed(KEYCODE_F9))
    {
        m_ambientIntensity += 0.05f;
        m_ambientIntensity = GetClampedZeroToOne(m_ambientIntensity);
        DebugAddMessage(Stringf("Ambient Intensity: (%.2f)", m_ambientIntensity), 5.f);
    }
}

//----------------------------------------------------------------------------------------------------
void Map::UpdateAllActors(float const deltaSeconds) const
{
    for (int i = 0; i < static_cast<int>(m_actors.size()); i++)
    {
        if (m_actors[i] != nullptr)
        {
            m_actors[i]->Update(deltaSeconds);
        }
    }
}

//----------------------------------------------------------------------------------------------------
void Map::CollideActors()
{
    for (int i = 0; i < static_cast<int>(m_actors.size()); ++i)
    {
        if (m_actors[i] == nullptr && !m_actors[i]->m_handle.IsValid()) continue;

        for (int j = i + 1; j < static_cast<int>(m_actors.size()); ++j)
        {
            if (m_actors[j] == nullptr && !m_actors[j]->m_handle.IsValid()) continue;

            CollideActors(m_actors[i], m_actors[j]);
        }
    }
}

//----------------------------------------------------------------------------------------------------
void Map::CollideActors(Actor* actorA,
                        Actor* actorB)
{
    // 1. If neither actor is movable, there will be no collision, so return.
    // if (!actorA->m_isMovable && !actorB->m_isMovable) return;

    // 2. Get actors' MinMaxZ range.
    FloatRange const actorAMinMaxZ = actorA->m_collisionCylinder.GetFloatRange();
    FloatRange const actorBMinMaxZ = actorB->m_collisionCylinder.GetFloatRange();

    // 3. If actors are not overlapping on their MinMaxZ range, there will be no collision, so return.
    if (!actorAMinMaxZ.IsOverlappingWith(actorBMinMaxZ))
    {
        return;
    }

    // 4. Calculate actors' positionXY and radius.
    Vec2        actorAPositionXY = Vec2(actorA->m_position.x, actorA->m_position.y);
    Vec2        actorBPositionXY = Vec2(actorB->m_position.x, actorB->m_position.y);
    float const actorARadius     = actorA->m_radius;
    float const actorBRadius     = actorB->m_radius;
    // PushDiscOutOfDisc2D(actorAPositionXY, actorARadius, actorBPositionXY, actorBRadius);
    PushDiscsOutOfEachOther2D(actorAPositionXY, actorARadius, actorBPositionXY, actorBRadius);
    // 5. Push movable actor out of immovable actor.
    // if (actorA->m_isMovable && !actorB->m_isMovable)
    // {
    // }
    // else if (actorB->m_isMovable && !actorA->m_isMovable)
    // {
    //     PushDiscOutOfDisc2D(actorBPositionXY, actorBRadius, actorAPositionXY, actorARadius);
    // }

    // 6. Update actors' position.
    actorA->m_position.x = actorAPositionXY.x;
    actorA->m_position.y = actorAPositionXY.y;
    actorB->m_position.x = actorBPositionXY.x;
    actorB->m_position.y = actorBPositionXY.y;
}

//----------------------------------------------------------------------------------------------------
void Map::CollideActorsWithMap() const
{
    for (int actorIndex = 0; actorIndex < static_cast<int>(m_actors.size()); ++actorIndex)
    {
        if (m_actors[actorIndex] != nullptr)
        {
            CollideActorWithMap(m_actors[actorIndex]);
        }
    }
}

//----------------------------------------------------------------------------------------------------
void Map::CollideActorWithMap(Actor* actor) const
{
    Vec3&         actorPosition   = actor->m_position;
    IntVec2 const actorTileCoords = GetTileCoordsFromWorldPos(actorPosition);

    // Push out of cardinal neighbors (NSEW) first
    PushActorOutOfTileIfSolid(actor, actorTileCoords + IntVec2(1, 0));
    PushActorOutOfTileIfSolid(actor, actorTileCoords + IntVec2(0, 1));
    PushActorOutOfTileIfSolid(actor, actorTileCoords + IntVec2(-1, 0));
    PushActorOutOfTileIfSolid(actor, actorTileCoords + IntVec2(0, -1));

    // Push out of diagonal neighbors second
    PushActorOutOfTileIfSolid(actor, actorTileCoords + IntVec2(1, 1));
    PushActorOutOfTileIfSolid(actor, actorTileCoords + IntVec2(-1, 1));
    PushActorOutOfTileIfSolid(actor, actorTileCoords + IntVec2(-1, -1));
    PushActorOutOfTileIfSolid(actor, actorTileCoords + IntVec2(1, -1));

    actorPosition.z = GetClamped(actorPosition.z, 0.f, 1.f - actor->m_height);
}

//----------------------------------------------------------------------------------------------------
void Map::PushActorOutOfTileIfSolid(Actor*         actor,
                                    IntVec2 const& tileCoords) const
{
    if (!IsTileSolid(tileCoords)) return;

    if (IsTileCoordsOutOfBounds(tileCoords)) return;

    // TODO: Swap check method for Sprinting if needed (PushCapsuleOutOfAABB2D/DoCapsuleAndAABB2Overlap2D)

    AABB3 const aabb3Box = GetTile(tileCoords.x, tileCoords.y)->m_bounds;
    AABB2 const aabb2Box = AABB2(Vec2(aabb3Box.m_mins.x, aabb3Box.m_mins.y), Vec2(aabb3Box.m_maxs.x, aabb3Box.m_maxs.y));

    Vec2 actorPositionXY = Vec2(actor->m_position.x, actor->m_position.y);

    PushDiscOutOfAABB2D(actorPositionXY, actor->m_radius, aabb2Box);

    actor->m_position.x = actorPositionXY.x;
    actor->m_position.y = actorPositionXY.y;
}

//----------------------------------------------------------------------------------------------------
void Map::Render() const
{
    RenderAllActors();
    RenderMap();
}

//----------------------------------------------------------------------------------------------------
void Map::RenderAllActors() const
{
    for (int i = 0; i < static_cast<int>(m_actors.size()); i++)
    {
        if (m_actors[i] != nullptr)
        {
            m_actors[i]->Render();
        }
    }
}

//----------------------------------------------------------------------------------------------------
void Map::RenderMap() const
{
    g_theRenderer->SetModelConstants();
    g_theRenderer->SetLightConstants(m_sunDirection, m_sunIntensity, m_ambientIntensity);
    g_theRenderer->SetBlendMode(eBlendMode::OPAQUE);
    g_theRenderer->SetRasterizerMode(eRasterizerMode::SOLID_CULL_BACK);
    g_theRenderer->SetSamplerMode(eSamplerMode::POINT_CLAMP);
    g_theRenderer->SetDepthMode(eDepthMode::READ_WRITE_LESS_EQUAL);
    g_theRenderer->BindTexture(m_texture);
    g_theRenderer->BindShader(m_shader);

    g_theRenderer->DrawVertexArray(m_vertexes, m_indexes);
}

//----------------------------------------------------------------------------------------------------
RaycastResult3D Map::RaycastAll(Vec3 const& startPosition,
                                Vec3 const& forwardNormal,
                                float const maxLength) const
{
    RaycastResult3D closestResult;
    float           closestLength = maxLength;

    IntVec2 startTileCoords = GetTileCoordsFromWorldPos(startPosition);

    if (!IsTileCoordsOutOfBounds(startTileCoords))
    {
        Tile const* startTile       = GetTile(startTileCoords);
        AABB3 const startTileBounds = startTile->m_bounds;

        if (startTileBounds.IsPointInside(startPosition) &&
            startTile->m_isSolid)
        {
            closestResult.m_didImpact      = false;
            closestResult.m_impactPosition = startPosition;
            closestResult.m_impactNormal   = -closestResult.m_rayForwardNormal;

            return closestResult;
        }
    }

    RaycastResult3D xyResult = RaycastWorldXY(startPosition, forwardNormal, maxLength);

    if (xyResult.m_didImpact &&
        xyResult.m_impactLength < closestLength)
    {
        closestResult = xyResult;
        closestLength = xyResult.m_impactLength;
    }

    RaycastResult3D zResult = RaycastWorldZ(startPosition, forwardNormal, maxLength);

    if (zResult.m_didImpact &&
        zResult.m_impactLength < closestLength)
    {
        closestResult = zResult;
        closestLength = zResult.m_impactLength;
    }

    RaycastResult3D actorResult = RaycastWorldActors(startPosition, forwardNormal, maxLength);

    if (actorResult.m_didImpact &&
        actorResult.m_impactLength < closestLength)
    {
        closestResult = actorResult;
        closestLength = actorResult.m_impactLength;
    }

    return closestResult;
}

//----------------------------------------------------------------------------------------------------
RaycastResult3D Map::RaycastWorldXY(Vec3 const& startPosition,
                                    Vec3 const& forwardNormal,
                                    float const maxLength) const
{
    // 1. Initialize raycastResult3D.
    RaycastResult3D result;
    result.m_rayStartPosition = startPosition;
    result.m_rayForwardNormal = forwardNormal;
    result.m_rayMaxLength     = maxLength;

    // 2. Calculate information for the ray's stepping while-loop.
    Vec3             currentPosition   = startPosition;
    float            currentLength     = 0.f;
    float constexpr  stepSize          = 0.01f;
    Vec3 const       rayMoveStep       = forwardNormal * stepSize;
    float const      rayMoveStepLength = rayMoveStep.GetLength();
    FloatRange const rangeWorldZ       = FloatRange(0.f, 1.f);

    // 3. While the ray is still stepping forward,
    while (currentLength < maxLength)
    {
        // 4. Step the ray forward.
        currentPosition += rayMoveStep;
        currentLength += rayMoveStepLength;

        // 5. If the current tile is not in the map, continue the loop.
        IntVec2 currentTileCoords = GetTileCoordsFromWorldPos(currentPosition);

        if (IsTileCoordsOutOfBounds(currentTileCoords))
        {
            continue;
        }

        Tile const* currentTile         = GetTile(currentTileCoords);
        AABB3 const currentTileBounds3D = currentTile->m_bounds;
        AABB2 const currentTileBounds2D = AABB2(Vec2(currentTileBounds3D.m_mins.x, currentTileBounds3D.m_mins.y), Vec2(currentTileBounds3D.m_maxs.x, currentTileBounds3D.m_maxs.y));
        Vec3 const  previousPosition    = currentPosition - forwardNormal * rayMoveStepLength;
        Vec2 const  previousPositionXY  = Vec2(previousPosition.x, previousPosition.y);

        if (IsTileSolid(currentTileCoords) &&
            rangeWorldZ.IsOnRange(currentPosition.z) &&
            !currentTileBounds2D.IsPointInside(previousPositionXY))
        {
            result.m_didImpact         = true;
            result.m_impactPosition    = currentPosition;
            IntVec2 const impactNormal = GetTileCoordsFromWorldPos(previousPosition) - GetTileCoordsFromWorldPos(currentPosition);
            result.m_impactNormal      = Vec3(impactNormal.x, impactNormal.y, 0);
            result.m_impactLength      = currentLength;

            return result;
        }
    }

    result.m_didImpact      = false;
    result.m_impactPosition = startPosition;
    result.m_impactNormal   = -forwardNormal;
    result.m_impactLength   = 0.f;

    return result;
}

//----------------------------------------------------------------------------------------------------
RaycastResult3D Map::RaycastWorldZ(Vec3 const& startPosition,
                                   Vec3 const& forwardNormal,
                                   float const maxLength) const
{
    // 1. Initialize raycastResult3D.
    RaycastResult3D result;
    result.m_didImpact        = false;
    result.m_impactPosition   = startPosition;
    result.m_impactNormal     = -forwardNormal;
    result.m_impactLength     = 0.f;
    result.m_rayStartPosition = startPosition;
    result.m_rayForwardNormal = forwardNormal;
    result.m_rayMaxLength     = maxLength;

    // 2. Calculate the Z portion of startPosition, forwardNormalZ, and maxLength.
    float const startPositionZ = startPosition.z;
    float const forwardNormalZ = forwardNormal.z;
    float const maxLengthZ     = forwardNormal.z * maxLength;

    // 3. If the ray only moves on XY surface, that means it would never impact,
    // return the initial raycastResult3D.
    if (forwardNormalZ == 0.f)
    {
        return result;
    }

    // 4. Calculate the scalar t (0 to 1) based on the ray's forwardNormalZ.
    float const oneOverMaxLengthZ = 1.f / maxLengthZ;
    float const t                 = forwardNormalZ > 0.f ? (1.f - startPositionZ) * oneOverMaxLengthZ : -startPositionZ * oneOverMaxLengthZ;

    // 5. If t is not within 0 to 1, which means the ray impacts before its startPosition or its endPosition,
    // return the initial raycastResult3D.
    if (t < 0.f || t > 1.f)
    {
        return result;
    }

    // 6. Calculate the impactLength and the impactPosition based on t.
    float const impactLength   = maxLength * t;
    Vec3 const  impactPosition = startPosition + forwardNormal * impactLength;

    // 7. If the impactPosition is outside the map, return with initial raycastResult3D.
    if (!IsPositionInBounds(impactPosition))
    {
        return result;
    }

    // RAY HIT
    result.m_didImpact      = true;
    result.m_impactPosition = impactPosition;
    result.m_impactNormal   = forwardNormalZ > 0.f ? -Vec3::Z_BASIS : Vec3::Z_BASIS;
    result.m_impactLength   = (forwardNormal * t * maxLength).GetLength();

    return result;
}


//----------------------------------------------------------------------------------------------------
RaycastResult3D Map::RaycastWorldActors(Vec3 const& startPosition,
                                        Vec3 const& forwardNormal,
                                        float const maxLength) const
{
    RaycastResult3D closestResult;
    float           closestDistance = maxLength;

    for (int i = 0; i < static_cast<int>(m_actors.size()); i++)
    {
        Cylinder3             cylinder3 = m_actors[i]->m_collisionCylinder;
        RaycastResult3D const result    = RaycastVsCylinderZ3D(startPosition,
                                                               forwardNormal, maxLength,
                                                               cylinder3.GetCenterPositionXY(),
                                                               cylinder3.GetFloatRange(),
                                                               cylinder3.m_radius);

        if (result.m_didImpact &&
            result.m_impactLength < closestDistance)
        {
            closestResult   = result;
            closestDistance = result.m_impactLength;
        }
    }

    return closestResult;
}

//----------------------------------------------------------------------------------------------------
// Spawn a specified actor according to the provided spawn info.
// Should find or add a slot in our actor list, increment our next uid, generate a handle, and construct and return the actor.
Actor* Map::SpawnActor(SpawnInfo const& spawnInfo)
{
    if (m_nextActorUID >= MAX_ACTOR_UID)
    {
        return nullptr;
    }

    unsigned int const newIndex = static_cast<unsigned int>(m_actors.size());
    Actor*             newActor = new Actor(spawnInfo);

    ActorHandle const handle = ActorHandle(m_nextActorUID, newIndex);
    newActor->m_handle       = handle;
    newActor->m_map          = this;

    m_actors.push_back(newActor);
    m_nextActorUID++;

    return newActor;
}

//----------------------------------------------------------------------------------------------------
// Dereference an actor handle and return an actor pointer.
// Get the index from the actor handle.
// If that slot in our list of actors is null, return null.
// If that slot in our list of actors contains an actor, check if that actor’s handle matches the input actor handle.
// If they do not match, return null.
// Otherwise, return the actor pointer at that index.
Actor* Map::GetActorByHandle(ActorHandle const handle) const
{
    if (!handle.IsValid())
    {
        return nullptr;
    }

    unsigned int const handleIndex = handle.GetIndex();

    if (handleIndex >= m_actors.size())
    {
        return nullptr;
    }

    Actor* storedActor = m_actors[handleIndex];

    if (storedActor == nullptr)
    {
        return nullptr;
    }

    if (storedActor->m_handle != handle)
    {
        return nullptr;
    }

    return storedActor;
}

Actor const* Map::GetActorByName(String const& name)
{
    for (Actor const* actor : m_actors)
    {
        if (actor != nullptr &&
            actor->m_handle.IsValid())
        {
            if (actor->m_definition->m_name == name)
            {
                return actor;
            }
        }
    }

    return nullptr;
}

void Map::GetActorsByName(std::vector<Actor*>& actorList, String const& name)
{
    for (Actor* actor : m_actors)
    {
        if (actor != nullptr &&
            actor->m_handle.IsValid())
        {
            if (actor->m_definition->m_name == name)
            {
                actorList.push_back(actor);
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------
// Delete any actors marked as destroyed.
void Map::DeleteDestroyedActor()
{
    for (Actor* actor : m_actors)
    {
        if (actor && actor->m_handle.IsValid() && actor->m_isGarbage)
        {
            unsigned int index = actor->m_handle.GetIndex();
            delete actor;
            m_actors[index] = nullptr;
        }
    }
}

//----------------------------------------------------------------------------------------------------
// Spawn a marine actor at a random spawn point and possess it with the player.
Actor* Map::SpawnPlayer(PlayerController* playerController)
{
    SpawnInfo spawnInfo;
    spawnInfo.m_name = "Marine";
    std::vector<Actor*> spawnPoints;
    GetActorsByName(spawnPoints, "SpawnPoint");
    Actor* spawnPoint       = spawnPoints[g_theRNG->RollRandomIntInRange(0, (int)spawnPoints.size() - 1)];
    spawnInfo.m_position    = spawnPoint->m_position;
    spawnInfo.m_orientation = spawnPoint->m_orientation;
    spawnInfo.m_velocity    = spawnPoint->m_velocity;
    Actor* playerActor      = SpawnActor(spawnInfo);
    playerController->m_map = this;
    return playerActor;
}

//----------------------------------------------------------------------------------------------------
// Search the actor list to find actors meeting the provided criteria.
Actor* Map::GetClosestVisibleEnemy(Actor* owner)
{
    return nullptr;
}

//----------------------------------------------------------------------------------------------------
// Have the player controller possess the next actor in the list that can be possessed.
void Map::DebugPossessNext() const
{
    PlayerController* playerController = m_game->GetPlayerController();

    if (playerController == nullptr)
    {
        return;
    }

    Actor const* playerControlledActor = playerController->GetActor();
    unsigned int startIndex            = 0;

    if (playerControlledActor != nullptr)
    {
        startIndex = playerControlledActor->m_handle.GetIndex() + 1;
    }

    unsigned int const actorCount = static_cast<unsigned int>(m_actors.size());
    if (actorCount == 0)
    {
        return;
    }

    for (unsigned int i = 0; i < actorCount; i++)
    {
        unsigned int const desiredIndex   = (startIndex + i) % actorCount;
        Actor*             potentialActor = m_actors[desiredIndex];

        if (potentialActor &&
            potentialActor->m_handle.IsValid() &&
            potentialActor->m_definition->m_canBePossessed)
        {
            playerController->Possess(potentialActor->m_handle);
            return;
        }
    }
}
