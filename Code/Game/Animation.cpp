#include "Animation.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/GameCommon.hpp"

Animation::Animation(XmlElement const& animationElement)
{
    m_name            = ParseXmlAttribute(animationElement, "name", m_name);
    m_cellCount       = ParseXmlAttribute(animationElement, "cellCount", m_cellCount);
    String shaderPath = ParseXmlAttribute(animationElement, "shader", "DEFAULT");
    m_shader          = g_theRenderer->CreateOrGetShaderFromFile(shaderPath.c_str(), eVertexType::VERTEX_PCU);
    m_spriteSheet     = new SpriteSheet(*g_theRenderer->CreateOrGetTextureFromFile(ParseXmlAttribute(animationElement, "spriteSheet", m_name).c_str()), m_cellCount);
    m_startFrame      = ParseXmlAttribute(animationElement, "startFrame", 0);
    m_endFrame        = ParseXmlAttribute(animationElement, "endFrame", 0);
    m_secondsPerFrame = ParseXmlAttribute(animationElement, "secondsPerFrame", m_secondsPerFrame);
    m_spriteAnim      = new SpriteAnimDefinition(*m_spriteSheet, m_startFrame, m_endFrame, 1.0f / m_secondsPerFrame, m_playbackType);
    printf("Animation::Animation    Create Animation: %s \n", m_name.c_str());
}

Animation::~Animation()
{
    // SafeDeletePointer(m_shader);
}

float Animation::GetAnimationLength()
{
    return m_spriteAnim->GetDuration();
}

const SpriteAnimDefinition* Animation::GetAnimationDefinition()
{
    return m_spriteAnim;
}
