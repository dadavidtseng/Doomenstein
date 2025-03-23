//----------------------------------------------------------------------------------------------------
// Map.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Map.hpp"

#include "GameCommon.hpp"
#include "Tile.hpp"
#include "TileDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/MapDefinition.hpp"

//----------------------------------------------------------------------------------------------------
Map::Map(Game*                owner,
         MapDefinition const& mapDef)
    : m_game(owner),
      m_definition(&mapDef)
{
    m_vertexes.reserve(10000);
    m_tiles.reserve(10000);

    m_dimensions = m_definition->m_image.GetDimensions();
    m_texture    = mapDef.m_spriteSheetTexture;

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
    unsigned int indexOffset = 0;
    IntVec2 spriteSheetCellCount = m_definition->m_spriteSheetCellCount;
    for (int i = 0; i < m_dimensions.x; ++i)
    {
        for (int j = 0; j < m_dimensions.y; ++j)
        {
            m_indexes.push_back(indexOffset);
            m_indexes.push_back(indexOffset + 1);
            m_indexes.push_back(indexOffset + 2);
            m_indexes.push_back(indexOffset);
            m_indexes.push_back(indexOffset + 2);
            m_indexes.push_back(indexOffset + 3);

            if (m_tiles[j + i * m_dimensions.y].m_name == "BrickWall")
            {
                float x = RangeMap(0, 0,(float)spriteSheetCellCount.x,0,1);
                float y = RangeMap(2, 0,(float)spriteSheetCellCount.y,0,1);
                AddVertsForAABB3D(m_vertexes, m_indexes, m_tiles[j + i * m_dimensions.y].m_bounds, Rgba8::WHITE, AABB2(Vec2(x,y), Vec2(x+1.f/(float)spriteSheetCellCount.x, y+1.f/(float)spriteSheetCellCount.y)));
            }

            indexOffset += 4;
        }
    }
}

//----------------------------------------------------------------------------------------------------
void Map::AddGeometryForWall(VertexList_PCUTBN& verts,
                             IndexList&         indexes,
                             AABB3 const&       bounds,
                             AABB2 const&       UVs) const
{
}

//----------------------------------------------------------------------------------------------------
void Map::AddGeometryForFloor(VertexList_PCUTBN& verts,
                              IndexList&         indexes,
                              AABB3 const&       bounds,
                              AABB2 const&       UVs) const
{
}

//----------------------------------------------------------------------------------------------------
void Map::AddGeometryForCeiling(VertexList_PCUTBN& verts,
                                IndexList&         indexes,
                                AABB3 const&       bounds,
                                AABB2 const&       UVs) const
{
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
    g_theRenderer->SetModelConstants();
    g_theRenderer->SetBlendMode(eBlendMode::OPAQUE);
    g_theRenderer->SetRasterizerMode(eRasterizerMode::SOLID_CULL_BACK);
    g_theRenderer->SetSamplerMode(eSamplerMode::POINT_CLAMP);
    g_theRenderer->SetDepthMode(eDepthMode::READ_WRITE_LESS_EQUAL);
    g_theRenderer->BindTexture(m_texture);

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
