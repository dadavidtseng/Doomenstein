//----------------------------------------------------------------------------------------------------
// Map.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Map.hpp"

#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/Game.hpp"
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

    m_lightCBO = g_theRenderer->CreateConstantBuffer(sizeof(LightConstants));
}

//----------------------------------------------------------------------------------------------------
Map::~Map()
{
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
                    m_tiles[j + i * m_dimensions.y].m_bounds = bounds;
                    m_tiles[j + i * m_dimensions.y].m_name   = tileDef->m_name;
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

            m_indexes.push_back(indexOffset);
            m_indexes.push_back(indexOffset + 1);
            m_indexes.push_back(indexOffset + 2);
            m_indexes.push_back(indexOffset);
            m_indexes.push_back(indexOffset + 2);
            m_indexes.push_back(indexOffset + 3);

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
bool Map::IsPositionInBounds(Vec3 position, float const tolerance) const
{
    return false;
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
        DebugAddMessage(Stringf("Sun Intensity: (%.2f)", m_sunIntensity), 5.f);
    }
    if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
    {
        m_sunIntensity += 0.05f;
        DebugAddMessage(Stringf("Sun Intensity: (%.2f)", m_sunIntensity), 5.f);
    }
    if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
    {
        m_ambientIntensity -= 0.05f;
        DebugAddMessage(Stringf("Ambient Intensity: (%.2f)", m_ambientIntensity), 5.f);
    }
    if (g_theInput->WasKeyJustPressed(KEYCODE_F9))
    {
        m_ambientIntensity += 0.05f;
        DebugAddMessage(Stringf("Ambient Intensity: (%.2f)", m_ambientIntensity), 5.f);
    }

    m_sunIntensity     = GetClampedZeroToOne(m_sunIntensity);
    m_ambientIntensity = GetClampedZeroToOne(m_ambientIntensity);
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
    LightConstants lightConstants;

    Vec3 const normalizedSunDirection = m_sunDirection.GetNormalized();

    lightConstants.SunDirection[0]  = normalizedSunDirection.x;
    lightConstants.SunDirection[1]  = normalizedSunDirection.y;
    lightConstants.SunDirection[2]  = normalizedSunDirection.z;
    lightConstants.SunIntensity     = m_sunIntensity;
    lightConstants.AmbientIntensity = m_ambientIntensity;

    g_theRenderer->CopyCPUToGPU(&lightConstants, sizeof(LightConstants), m_lightCBO);
    g_theRenderer->BindConstantBuffer(Renderer::k_lightConstantSlot, m_lightCBO);

    g_theRenderer->SetModelConstants();
    g_theRenderer->SetBlendMode(eBlendMode::OPAQUE);
    g_theRenderer->SetRasterizerMode(eRasterizerMode::SOLID_CULL_BACK);
    g_theRenderer->SetSamplerMode(eSamplerMode::POINT_CLAMP);
    g_theRenderer->SetDepthMode(eDepthMode::READ_WRITE_LESS_EQUAL);
    g_theRenderer->BindTexture(m_texture);
    g_theRenderer->BindShader(m_shader);

    g_theRenderer->DrawVertexArray(m_vertexes, m_indexes);
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
