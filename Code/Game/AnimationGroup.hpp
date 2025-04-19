//----------------------------------------------------------------------------------------------------
// AnimationGroup.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include <map>

#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Math/Vec3.hpp"

enum class SpriteAnimPlaybackType;
class SpriteAnimDefinition;

class AnimationGroup
{
public:
    AnimationGroup(XmlElement const& animationGroupElement, SpriteSheet const& spriteSheet);

    /// Getter
    SpriteAnimDefinition const& GetSpriteAnimation(Vec3 const& direction) const;
    float                       GetAnimationLength();
    int                         GetAnimationTotalFrame();

    std::string                          m_name            = "Default";
    float                                m_scaleBySpeed    = true;
    float                                m_secondsPerFrame = 0.25f;
    SpriteAnimPlaybackType               m_playbackType    = SpriteAnimPlaybackType::LOOP;
    SpriteSheet const&                   m_spriteSheet;
    std::map<Vec3, SpriteAnimDefinition> m_animations;
};
