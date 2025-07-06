//----------------------------------------------------------------------------------------------------
// HUD.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Gameplay/HUD.hpp"

#include "Engine/Renderer/Renderer.hpp"
#include "Game/Framework/Animation.hpp"
#include "Game/Framework/GameCommon.hpp"

//----------------------------------------------------------------------------------------------------
HUD::HUD(XmlElement const& hudElement)
{
    m_shaderName            = ParseXmlAttribute(hudElement, "shader", "DEFAULT");
    m_shader                = g_theRenderer->CreateOrGetShaderFromFile(m_shaderName.c_str(), eVertexType::VERTEX_PCU);
    m_reticuleSize          = ParseXmlAttribute(hudElement, "reticleSize", m_reticuleSize);
    m_spriteSize            = ParseXmlAttribute(hudElement, "spriteSize", m_spriteSize);
    m_spritePivot           = ParseXmlAttribute(hudElement, "spritePivot", m_spritePivot);
    m_baseTexturePath       = ParseXmlAttribute(hudElement, "baseTexture", m_baseTexturePath);
    m_m_reticuleTexturePath = ParseXmlAttribute(hudElement, "reticleTexture", m_m_reticuleTexturePath);

    m_baseTexture     = g_theRenderer->CreateOrGetTextureFromFile(m_baseTexturePath.c_str());
    m_reticuleTexture = g_theRenderer->CreateOrGetTextureFromFile(m_m_reticuleTexturePath.c_str());

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
}

//----------------------------------------------------------------------------------------------------
Animation* HUD::GetAnimationByName(String const& animationName)
{
    for (Animation& animation : m_animations)
    {
        if (animation.GetName() == animationName)
        {
            return &animation;
        }
    }
    return nullptr;
}

//----------------------------------------------------------------------------------------------------
std::vector<Animation>& HUD::GetAnimations()
{
    return m_animations;
}
