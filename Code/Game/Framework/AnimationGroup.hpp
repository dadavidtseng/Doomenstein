//----------------------------------------------------------------------------------------------------
// AnimationGroup.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include <map>

#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Math/Vec3.hpp"

//----------------------------------------------------------------------------------------------------
class AnimationGroup
{
public:
    AnimationGroup(XmlElement const& animationGroupElement, SpriteSheet const& spriteSheet);

    // Accessors (const methods)
    String                      GetName() const;
    SpriteAnimDefinition const& GetSpriteAnimation(Vec3 const& direction) const;
    float                       GetAnimationLength() const;
    int                         GetAnimationTotalFrame() const;

private:
    String                               m_name            = "DEFAULT";
    float                                m_scaleBySpeed    = false;
    float                                m_secondsPerFrame = 0.f;
    SpriteAnimPlaybackType               m_playbackType    = SpriteAnimPlaybackType::LOOP;
    std::map<Vec3, SpriteAnimDefinition> m_animationDict;
};
