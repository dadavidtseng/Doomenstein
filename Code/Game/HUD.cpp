#include "Game/HUD.hpp"

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Game/GameCommon.hpp"

HUD::HUD(XmlElement const& hudElement)
{
    m_shaderName           = ParseXmlAttribute(hudElement, "shader", "DEFAULT");
    m_shader               = g_theRenderer->CreateOrGetShaderFromFile(m_shaderName.c_str(), eVertexType::VERTEX_PCU);
    m_reticleSize          = ParseXmlAttribute(hudElement, "reticleSize", m_reticleSize);
    m_spriteSize           = ParseXmlAttribute(hudElement, "spriteSize", m_spriteSize);
    m_spritePivot          = ParseXmlAttribute(hudElement, "spritePivot", m_spritePivot);
    m_baseTexturePath      = ParseXmlAttribute(hudElement, "baseTexture", m_baseTexturePath);
    m_m_reticleTexturePath = ParseXmlAttribute(hudElement, "reticleTexture", m_m_reticleTexturePath);

    m_baseTexture    = g_theRenderer->CreateOrGetTextureFromFile(m_baseTexturePath.c_str());
    m_reticleTexture = g_theRenderer->CreateOrGetTextureFromFile(m_m_reticleTexturePath.c_str());

    if (hudElement.ChildElementCount() > 0)
    {
        XmlElement const* element = hudElement.FirstChildElement();
        while (element != nullptr)
        {
            Animation animation = Animation(*element);
            m_animations.push_back(animation);
            element = element->NextSiblingElement();
        }
    }
    printf("Hud::Hud    Create Hud with base texture: %s\n", m_baseTexturePath.c_str());
}

HUD::~HUD()
{
    // SafeDeletePointer(m_shader);
}

Animation* HUD::GetAnimationByName(std::string const& animationName)
{
    for (Animation& animation : m_animations)
    {
        if (animation.m_name == animationName)
        {
            return &animation;
        }
    }
    return nullptr;
}

std::vector<Animation>& HUD::GetAnimations()
{
    return m_animations;
}
