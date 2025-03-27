//----------------------------------------------------------------------------------------------------
// Map.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Map.hpp"

#include "Actor.hpp"
#include "Game.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Game/GameCommon.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/Tile.hpp"
#include "Game/TileDefinition.hpp"


//----------------------------------------------------------------------------------------------------
Map::Map(Game*                owner,
         MapDefinition const& mapDef)
    : m_game(owner),
      m_definition(&mapDef)
{
    m_dimensions = m_definition->m_image.GetDimensions();

    m_vertexes.reserve(sizeof(AABB3) * m_dimensions.x * m_dimensions.y);
    m_tiles.reserve(static_cast<unsigned int>(m_dimensions.x * m_dimensions.y));
    m_actors.reserve(4);

    m_texture = m_definition->m_spriteSheetTexture;
    m_shader  = m_definition->m_shader;

    CreateBuffers();
    CreateTiles();
    CreateGeometry();

    // m_shader             = g_theRenderer->CreateOrGetShaderFromFile(shaderFilePath.c_str(), eVertexType::VERTEX_PCUTBN);
    // m_spriteSheetTexture = g_theRenderer->CreateOrGetTextureFromFile(spriteSheetTextureFilePath.c_str());
    //
    // if (XmlElement const* spawnInfosElement = mapDefElement.FirstChildElement("SpawnInfos"))
    // {
    //     for (XmlElement const* spawnElement = spawnInfosElement->FirstChildElement("SpawnInfo");
    //          spawnElement != nullptr;
    //          spawnElement = spawnElement->NextSiblingElement("SpawnInfo"))
    //     {
    //         // m_spawnInfos.emplace_back(spawnElement);
    //
    //         String actor = ParseXmlAttribute(*spawnElement, "actor", "Unnamed");
    //         String position = ParseXmlAttribute(*spawnElement, "position", "Unnamed");
    //         String orientation = ParseXmlAttribute(*spawnElement, "orientation", "Unnamed");
    //         DebuggerPrintf((actor + "\n").c_str());
    //         DebuggerPrintf((position + "\n").c_str());
    //         DebuggerPrintf((orientation + "\n").c_str());
    //     }
    // }

    m_actors.push_back(new Actor(Vec3(7.5f, 8.5f, 0.25f), EulerAngles::ZERO, 0.35f, 0.75f, false, Rgba8::RED));
    m_actors.push_back(new Actor(Vec3(8.5f, 8.5f, 0.125f), EulerAngles::ZERO, 0.35f, 0.75f, false, Rgba8::RED));
    m_actors.push_back(new Actor(Vec3(9.5f, 8.5f, 0.f), EulerAngles::ZERO, 0.35f, 0.75f, false, Rgba8::RED));
    m_actors.push_back(new Actor(Vec3(5.5f, 8.5f, 0.f), EulerAngles::ZERO, 0.0625f, 0.125f, true, Rgba8::BLUE));
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
                if (m_definition->m_image.GetTexelColor(coords) == tileDef->m_mapImagePixelColor)
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
    IntVec2 const     spriteSheetCellCount = m_definition->m_spriteSheetCellCount;
    SpriteSheet const spriteSheet          = SpriteSheet(*m_definition->m_spriteSheetTexture, spriteSheetCellCount);

    for (int i = 0; i < m_dimensions.x; ++i)
    {
        for (int j = 0; j < m_dimensions.y; ++j)
        {
            AABB3 const bounds = AABB3(Vec3(i, j, 0), Vec3(i + 1, j + 1, 1));

            IntVec2 currentTileCoords = IntVec2(i, j);
            AABB2   wallUVs, floorUVs, ceilingUVs;

            for (TileDefinition const* tileDef : TileDefinition::s_tileDefinitions)
            {
                if (m_definition->m_image.GetTexelColor(currentTileCoords) == tileDef->m_mapImagePixelColor)
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
        position.x < (float)m_dimensions.x + tolerance &&
        position.y < (float)m_dimensions.y + tolerance;
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
void Map::Update()
{
    for (int i = 0; i < (int)m_actors.size(); i++)
    {
        if (m_actors[i] != nullptr)
        {
            m_actors[i]->Update();
        }
    }

    UpdateFromKeyboard();
    CollideActors();
    CollideActorsWithMap();
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
void Map::CollideActors()
{
    int actorCount = (int)m_actors.size();

    for (int i = 0; i < actorCount; ++i)
    {
        Actor* actorA = m_actors[i];
        if (actorA == nullptr) continue;

        for (int j = i + 1; j < actorCount; ++j)
        {
            Actor* actorB = m_actors[j];
            if (actorB == nullptr) continue;

            // 呼叫 CollideActors 來解決碰撞
            CollideActors(actorA, actorB);
        }
    }
}

//----------------------------------------------------------------------------------------------------
void Map::CollideActors(Actor* actorA, Actor* actorB)
{
    if (!actorA->m_isMovable &&!actorB->m_isMovable) return;

    // 檢查 z 軸是否重疊

    FloatRange const actorAMinMaxZ = actorA->m_cylinder.GetFloatRange();
    FloatRange const actorBMinMaxZ = actorB->m_cylinder.GetFloatRange();

    if (!actorAMinMaxZ.IsOverlappingWith(actorBMinMaxZ))
    {
        return; // 沒有在相同 z 軸範圍內，不發生碰撞
    }

    // 取得 2D 位置
    Vec2 posA = Vec2(actorA->m_position.x, actorA->m_position.y);
    Vec2 posB = Vec2(actorB->m_position.x, actorB->m_position.y);

    if (actorA->m_isMovable&&!actorB->m_isMovable)
    {
        PushDiscOutOfDisc2D(posA, actorA->m_radius, posB, actorB->m_radius);
    }
    else if (actorB->m_isMovable&&!actorA->m_isMovable)
    {
        PushDiscOutOfDisc2D(posB, actorB->m_radius, posA, actorA->m_radius);
    }
    // 處理圓形碰撞


    // 更新 Actor 位置
    actorA->m_position.x = posA.x;
    actorA->m_position.y = posA.y;
    actorB->m_position.x = posB.x;
    actorB->m_position.y = posB.y;
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
    IntVec2 const actorTileCoords = GetTileCoordsFromWorldPos(actor->m_position);

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
}

//----------------------------------------------------------------------------------------------------
void Map::PushActorOutOfTileIfSolid(Actor* actor, IntVec2 const& tileCoords) const
{
    if (!IsTileSolid(tileCoords)) return;

    if (IsTileCoordsOutOfBounds(tileCoords)) return;

    AABB3 const aabb3Box        = GetTile(tileCoords.x, tileCoords.y)->m_bounds;
    AABB2 const aabb2Box        = AABB2(Vec2(aabb3Box.m_mins.x, aabb3Box.m_mins.y), Vec2(aabb3Box.m_maxs.x, aabb3Box.m_maxs.y));
    Vec2        actorPositionXY = Vec2(actor->m_position.x, actor->m_position.y);

    PushDiscOutOfAABB2D(actorPositionXY, actor->m_radius, aabb2Box);

    actor->m_position.x = actorPositionXY.x;
    actor->m_position.y = actorPositionXY.y;
}

//----------------------------------------------------------------------------------------------------
void Map::Render() const
{
    for (int i = 0; i < (int)m_actors.size(); i++)
    {
        if (m_actors[i])
        {
            m_actors[i]->Render();
        }
    }

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
RaycastResult3D Map::RaycastAll(Vec3 const& start,
                                Vec3 const& direction,
                                float const distance) const
{
    RaycastResult3D closestResult;
    float           closestDistance = distance;

    RaycastResult3D xyResult = RaycastWorldXY(start, direction, distance);
    if (xyResult.m_didImpact && xyResult.m_impactLength < closestDistance)
    {
        closestResult   = xyResult;
        closestDistance = xyResult.m_impactLength;
    }

    RaycastResult3D zResult = RaycastWorldZ(start, direction, distance);
    if (zResult.m_didImpact && zResult.m_impactLength < closestDistance)
    {
        closestResult   = zResult;
        closestDistance = zResult.m_impactLength;
    }

    RaycastResult3D actorResult = RaycastWorldActors(start, direction, distance);
    if (actorResult.m_didImpact && actorResult.m_impactLength < closestDistance)
    {
        closestResult   = actorResult;
        closestDistance = actorResult.m_impactLength;
    }

    return closestResult;
}

//----------------------------------------------------------------------------------------------------
RaycastResult3D Map::RaycastWorldXY(Vec3 const& start,
                                    Vec3 const& direction,
                                    float const distance) const
{
    RaycastResult3D result;
    result.m_rayStartPosition = start;
    result.m_rayForwardNormal = direction.GetNormalized();
    result.m_rayMaxLength     = distance;

    Vec3  rayStep         = result.m_rayForwardNormal * 0.01f; // 每次移動的步長
    Vec3  currentPosition = start;
    float traveledDist    = 0.f;

    while (traveledDist < distance)
    {
        currentPosition += rayStep;
        traveledDist += rayStep.GetLength();

        IntVec2 tileCoords = IntVec2(RoundDownToInt(currentPosition.x), RoundDownToInt(currentPosition.y));

        // 超出邊界時的處理
        if (IsTileCoordsOutOfBounds(tileCoords))
        {
            result.m_didImpact      = true;
            result.m_impactPosition = currentPosition;
            result.m_impactLength   = traveledDist;
            result.m_impactNormal   = -result.m_rayForwardNormal;
            return result;
        }

        // 碰撞到牆壁（非水面）時的處理
        if (IsTileSolid(tileCoords))
        {
            result.m_didImpact      = true;
            result.m_impactPosition = currentPosition;
            result.m_impactLength   = traveledDist;
            // 計算 impactNormal
            Vec3 prePosition = currentPosition - result.m_rayForwardNormal * rayStep.GetLength();;

            IntVec2 impactNormal = GetTileCoordsFromWorldPos(prePosition) - GetTileCoordsFromWorldPos(currentPosition);

            result.m_impactNormal = Vec3(impactNormal.x, impactNormal.y, 0);

            return result;
        }
    }

    result.m_didImpact = false;
    return result;
}

//----------------------------------------------------------------------------------------------------
RaycastResult3D Map::RaycastWorldZ(Vec3 const& start,
                                   Vec3 const& forwardNormal,
                                   float const distance) const
{
    RaycastResult3D result;
    result.m_rayStartPosition = start;
    result.m_rayForwardNormal = forwardNormal;
    result.m_rayMaxLength     = distance;

    float vz = forwardNormal.z * distance;
    float Sz = start.z;
    float t  = 0.f;

    // 射線向上，檢測天花板 (z = 1)
    if (vz > 0.f)
    {
        t = (1.f - Sz) / vz;
        if (t >= 0.f && t <= 1.f)
        {
            result.m_impactPosition = start + forwardNormal * t * distance;
            result.m_impactNormal   = -Vec3::Z_BASIS;
            result.m_impactLength   = (forwardNormal * t * distance).GetLength();
            result.m_didImpact      = true;
            return result;
        }
    }
    // 射線向下，檢測地板 (z = 0)
    else if (vz < 0.f)
    {
        t = (-Sz) / vz;
        if (t >= 0.f && t <= 1.f)
        {
            result.m_impactPosition = start + forwardNormal * t * distance;
            result.m_impactNormal   = Vec3::Z_BASIS;
            result.m_impactLength   = (forwardNormal * t * distance).GetLength();
            result.m_didImpact      = true;
            return result;
        }
    }

    // 若 vz == 0 或 t 不在範圍內，則無碰撞
    result.m_didImpact = false;
    return result;

    return {};
}

//----------------------------------------------------------------------------------------------------
RaycastResult3D Map::RaycastWorldActors(Vec3 const& start,
                                        Vec3 const& direction,
                                        float const distance) const
{
    RaycastResult3D closestResult;
    float           closestDistance = distance;

    for (int i = 0; i < static_cast<int>(m_actors.size()); i++)
    {
        Cylinder3             cylinder3 = m_actors[i]->m_cylinder;
        RaycastResult3D const result    = RaycastVsCylinderZ3D(start, direction, distance,
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
