//----------------------------------------------------------------------------------------------------
// HUD.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include <vector>

#include "Game/Animation.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/Vec2.hpp"

class Texture;
class Shader;

class HUD
{
public:
    explicit HUD(XmlElement const& hudElement);
    ~HUD();
    Animation* GetAnimationByName(std::string const& animationName);
    std::vector<Animation>& GetAnimations();
public:
    std::string m_name           = "Default";
    Shader*     m_shader         = nullptr;
    Texture*    m_baseTexture    = nullptr;
    Texture*    m_reticleTexture = nullptr;
    IntVec2     m_reticleSize;
    IntVec2     m_spriteSize;
    Vec2        m_spritePivot;

private:
    std::string            m_shaderName           = "Default";
    std::string            m_baseTexturePath      = "";
    std::string            m_m_reticleTexturePath = "";
    std::vector<Animation> m_animations;
};
