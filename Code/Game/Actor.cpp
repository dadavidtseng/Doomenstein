//----------------------------------------------------------------------------------------------------
// Actor.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Actor.hpp"

#include "GameCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Renderer/Renderer.hpp"

//----------------------------------------------------------------------------------------------------
Actor::Actor(Vec3 const&        position,
             EulerAngles const& orientation)
    : m_position(position),
      m_orientation(orientation)
{
    m_cylinder = Cylinder3(m_position, m_position + Vec3::Z_BASIS, 10.f);
}

//----------------------------------------------------------------------------------------------------
void Actor::Update()
{
}

//----------------------------------------------------------------------------------------------------
void Actor::Render() const
{
    VertexList_PCU verts;

    // AddVertsForCylinder3D(verts, m_cylinder.m_startPosition,m_cylinder.m_endPosition, 3.f);
    AddVertsForCylinder3D(verts, m_position, m_position + Vec3::Z_BASIS, 10.f);
    // AddVertsForAABB3D(verts, AABB3::ZERO_TO_ONE);
    g_theRenderer->SetModelConstants(GetModelToWorldTransform(),Rgba8::RED);
    g_theRenderer->SetBlendMode(eBlendMode::OPAQUE);
    g_theRenderer->SetRasterizerMode(eRasterizerMode::SOLID_CULL_NONE);
    g_theRenderer->SetSamplerMode(eSamplerMode::POINT_CLAMP);
    g_theRenderer->SetDepthMode(eDepthMode::READ_WRITE_LESS_EQUAL);
    g_theRenderer->BindTexture(nullptr);
    g_theRenderer->BindShader(g_theRenderer->CreateOrGetShaderFromFile("Default", eVertexType::VERTEX_PCU));

    g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
}

Mat44 Actor::GetModelToWorldTransform() const
{
    Mat44 m2w;

    m2w.SetTranslation3D(m_position);
    m2w.Append(m_orientation.GetAsMatrix_IFwd_JLeft_KUp());

    return m2w;
}
