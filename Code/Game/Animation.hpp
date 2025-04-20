#pragma once
#include "Engine/Core/Timer.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

class SpriteSheet;

class Animation
{
public:
    Animation(XmlElement const& animationElement);
    ~Animation();
    float GetAnimationLength();

    const SpriteAnimDefinition* GetAnimationDefinition();

public:
    std::string m_name;

private:
    IntVec2 m_cellCount;
    float   m_secondsPerFrame = 0.f;
    int     m_startFrame      = 0;
    int     m_endFrame        = 0;

    Shader*                m_shader       = nullptr;
    const SpriteSheet*     m_spriteSheet  = nullptr;
    SpriteAnimDefinition*  m_spriteAnim   = nullptr;
    SpriteAnimPlaybackType m_playbackType = SpriteAnimPlaybackType::ONCE;
};
