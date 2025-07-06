//----------------------------------------------------------------------------------------------------
// AnimationGroup.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Framework/AnimationGroup.hpp"

#include "Engine/Math/MathUtils.hpp"

//----------------------------------------------------------------------------------------------------
AnimationGroup::AnimationGroup(XmlElement const&  animationGroupElement,
                               SpriteSheet const& spriteSheet)
{
    m_name              = ParseXmlAttribute(animationGroupElement, "name", m_name);
    m_scaleBySpeed      = ParseXmlAttribute(animationGroupElement, "scaleBySpeed", m_scaleBySpeed);
    m_secondsPerFrame   = ParseXmlAttribute(animationGroupElement, "secondsPerFrame", m_secondsPerFrame);
    String playbackMode = "Loop";
    playbackMode        = ParseXmlAttribute(animationGroupElement, "playbackMode", playbackMode);
    if (playbackMode == "Loop") m_playbackType = SpriteAnimPlaybackType::LOOP;
    if (playbackMode == "Once") m_playbackType = SpriteAnimPlaybackType::ONCE;
    if (playbackMode == "Pingpong") m_playbackType = SpriteAnimPlaybackType::PINGPONG;

    if (animationGroupElement.ChildElementCount() > 0)
    {
        XmlElement const* element = animationGroupElement.FirstChildElement();

        while (element != nullptr)
        {
            // Facing vector for a specific animation.
            // This is the direction from the viewer to the actor, relative to the basis of the actor.
            // Not normalized, must be normalized in code when after reading from file.
            Vec3                 directionVector  = ParseXmlAttribute(*element, "vector", Vec3::ZERO);
            XmlElement const*    animationElement = element->FirstChildElement();
            int const            startFrame       = ParseXmlAttribute(*animationElement, "startFrame", 0);
            int const            endFrame         = ParseXmlAttribute(*animationElement, "endFrame", 0);
            SpriteAnimDefinition animation        = SpriteAnimDefinition(spriteSheet, startFrame, endFrame, 1.f / m_secondsPerFrame, m_playbackType);
            m_animationDict.insert(std::make_pair(directionVector.GetNormalized(), animation));
            element = element->NextSiblingElement();
        }
    }
}

//----------------------------------------------------------------------------------------------------
String AnimationGroup::GetName() const
{
    return m_name;
}

//----------------------------------------------------------------------------------------------------
SpriteAnimDefinition const& AnimationGroup::GetSpriteAnimation(Vec3 const& direction) const
{
    Vec3  offsetMin     = direction;
    float directionScalar = -FLT_MAX;

    for (std::pair<Vec3 const, SpriteAnimDefinition> const& animation : m_animationDict)
    {
        float const scalar = DotProduct3D(direction, animation.first);

        if (scalar > directionScalar)
        {
            directionScalar = scalar;
            offsetMin     = animation.first;
        }
    }

    return m_animationDict.at(offsetMin);
}

//----------------------------------------------------------------------------------------------------
float AnimationGroup::GetAnimationLength() const
{
    for (std::pair<Vec3 const, SpriteAnimDefinition> const& animation : m_animationDict)
    {
        float const totalLength = animation.second.GetDuration();

        if (totalLength > 0.f) return totalLength;
    }

    return -1;
}

//----------------------------------------------------------------------------------------------------
int AnimationGroup::GetAnimationTotalFrame() const
{
    for (std::pair<Vec3 const, SpriteAnimDefinition> const& animation : m_animationDict)
    {
        int const totalFrame = animation.second.GetTotalFrameInCycle();

        if (totalFrame > 0) return totalFrame;
    }

    return -1;
}
