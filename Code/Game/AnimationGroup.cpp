//----------------------------------------------------------------------------------------------------
// AnimationGroup.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/AnimationGroup.hpp"

#include "Engine/Math/MathUtils.hpp"

//----------------------------------------------------------------------------------------------------
AnimationGroup::AnimationGroup(XmlElement const& animationGroupElement, SpriteSheet const& spriteSheet)
    : m_spriteSheet(spriteSheet)
{
    m_name                   = ParseXmlAttribute(animationGroupElement, "name", m_name);
    m_scaleBySpeed           = ParseXmlAttribute(animationGroupElement, "scaleBySpeed", m_scaleBySpeed);
    m_secondsPerFrame        = ParseXmlAttribute(animationGroupElement, "secondsPerFrame", m_secondsPerFrame);
    std::string playbackMode = "Loop";
    playbackMode             = ParseXmlAttribute(animationGroupElement, "playbackMode", playbackMode);
    if (playbackMode == "Loop")
    {
        m_playbackType = SpriteAnimPlaybackType::LOOP;
    }
    if (playbackMode == "Once")
    {
        m_playbackType = SpriteAnimPlaybackType::ONCE;
    }
    if (playbackMode == "Pingpong")
    {
        m_playbackType = SpriteAnimPlaybackType::PINGPONG;
    }
    if (animationGroupElement.ChildElementCount() > 0)
    {
        XmlElement const* element = animationGroupElement.FirstChildElement();
        while (element != nullptr)
        {
            Vec3                 directionVector  = ParseXmlAttribute(*element, "vector", Vec3::ZERO);
            XmlElement const*    animationElement = element->FirstChildElement();
            int                  startFrame       = ParseXmlAttribute(*animationElement, "startFrame", 0);
            int                  endFrame         = ParseXmlAttribute(*animationElement, "endFrame", 0);
            SpriteAnimDefinition animation        = SpriteAnimDefinition(spriteSheet, startFrame, endFrame, 1.0f / m_secondsPerFrame, m_playbackType);
            m_animations.insert(std::make_pair(directionVector.GetNormalized(), animation)); // Be-careful that
            element = element->NextSiblingElement();
        }
    }
}

SpriteAnimDefinition const& AnimationGroup::GetSpriteAnimation(Vec3 direction)
{
    Vec3  leastOffset     = direction;
    float directionScalar = -FLT_MAX;
    for (std::pair<const Vec3, SpriteAnimDefinition>& animation : m_animations)
    {
        float scalar = DotProduct3D(direction, animation.first);
        if (scalar > directionScalar)
        {
            directionScalar = scalar;
            leastOffset     = animation.first;
        }
    }
    return m_animations.at(leastOffset);
}

float AnimationGroup::GetAnimationLength()
{
    for (std::pair<const Vec3, SpriteAnimDefinition>& animation : m_animations)
    {
        float const totalLength = animation.second.GetDuration();
        if (totalLength > 0.f) return totalLength;
    }
    return -1;
}

int AnimationGroup::GetAnimationTotalFrame()
{
    int totalFrame = -1;
    for (std::pair<const Vec3, SpriteAnimDefinition>& animation : m_animations)
    {
        totalFrame = animation.second.GetTotalFrameInCycle();
        if (totalFrame > 0) return totalFrame;
    }
    return -1;
}
