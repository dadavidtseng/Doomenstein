//----------------------------------------------------------------------------------------------------
// Animation.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

//----------------------------------------------------------------------------------------------------
class Animation
{
public:
    explicit Animation(XmlElement const& element);

    float                       GetAnimationLength() const;
    SpriteAnimDefinition const* GetAnimationDefinition() const;

    String GetName() const;

private:
    String m_name = "DEFAULT";                                                  // Name of the animation, used in code when selecting animations to play.
    IntVec2                m_cellCount;                                         // Weapon sprite sheet grid dimensions.
    float                  m_secondsPerFrame = 1.f;                             // Seconds per frame for this animation.
    int                    m_startFrame      = 0;                               // Start frame for the animation.
    int                    m_endFrame        = 0;                               // End frame for the animation.
    Shader*                m_shader          = nullptr;                         // Shader to use when rendering the weapon sprite.
    SpriteSheet const*     m_spriteSheet     = nullptr;                         // Sprite sheet containing the weapon animations.
    SpriteAnimDefinition*  m_spriteAnim      = nullptr;
    SpriteAnimPlaybackType m_playbackType    = SpriteAnimPlaybackType::ONCE;
};
