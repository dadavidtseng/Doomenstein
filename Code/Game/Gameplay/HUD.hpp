//----------------------------------------------------------------------------------------------------
// HUD.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include <vector>

//----------------------------------------------------------------------------------------------------
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"

//-Forward-Declaration--------------------------------------------------------------------------------
class Animation;
class Shader;
class Texture;

//----------------------------------------------------------------------------------------------------
class HUD
{
public:
    explicit HUD(XmlElement const& hudElement);

    Animation*              GetAnimationByName(String const& animationName);
    std::vector<Animation>& GetAnimations();

    String   m_name            = "DEFAULT";
    Shader*  m_shader          = nullptr;                       // Shader to use when rendering the base HUD texture and reticule.
    Texture* m_baseTexture     = nullptr;                       // Sprite to be rendered for the base HUD texture.
    Texture* m_reticuleTexture = nullptr;                       // Sprite to be rendered for the reticule.
    IntVec2  m_reticuleSize    = IntVec2::ONE;                  // Size at which to render the reticule, in pixels.
    IntVec2  m_spriteSize      = IntVec2::ZERO;                 // Size at which the weapon sprite should be rendered, in pixels.
    Vec2     m_spritePivot     = Vec2(0.5f, 0.f);   // Pivot for centering and positioning the

private:
    String                 m_shaderName            = "DEFAULT";
    String                 m_baseTexturePath       = "DEFAULT";
    String                 m_m_reticuleTexturePath = "DEFAULT";
    std::vector<Animation> m_animations;
};
