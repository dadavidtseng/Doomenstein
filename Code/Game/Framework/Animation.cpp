//----------------------------------------------------------------------------------------------------
// Animation.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Framework/Animation.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/Framework/GameCommon.hpp"

//----------------------------------------------------------------------------------------------------
Animation::Animation(XmlElement const& element)
{
    m_name                  = ParseXmlAttribute(element, "name", m_name);
    m_cellCount             = ParseXmlAttribute(element, "cellCount", m_cellCount);
    String const shaderPath = ParseXmlAttribute(element, "shader", "DEFAULT");
    m_shader                = g_theRenderer->CreateOrGetShaderFromFile(shaderPath.c_str(), eVertexType::VERTEX_PCU);
    m_spriteSheet           = new SpriteSheet(*g_theRenderer->CreateOrGetTextureFromFile(ParseXmlAttribute(element, "spriteSheet", m_name).c_str()), m_cellCount);
    m_startFrame            = ParseXmlAttribute(element, "startFrame", m_startFrame);
    m_endFrame              = ParseXmlAttribute(element, "endFrame", m_endFrame);
    m_secondsPerFrame       = ParseXmlAttribute(element, "secondsPerFrame", m_secondsPerFrame);
    m_spriteAnim            = new SpriteAnimDefinition(*m_spriteSheet, m_startFrame, m_endFrame, 1.f / m_secondsPerFrame, m_playbackType);
}

//----------------------------------------------------------------------------------------------------
float Animation::GetAnimationLength() const
{
    return m_spriteAnim->GetDuration();
}

//----------------------------------------------------------------------------------------------------
SpriteAnimDefinition const* Animation::GetAnimationDefinition() const
{
    return m_spriteAnim;
}

//----------------------------------------------------------------------------------------------------
String Animation::GetName() const
{
    return m_name;
}
