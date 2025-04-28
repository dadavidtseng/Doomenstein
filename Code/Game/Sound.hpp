//----------------------------------------------------------------------------------------------------
// Sound.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include <string>

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/XmlUtils.hpp"

///
/// Different from FMod::Sound, this class Encapsulate sound name, file path and SoundID from
/// FMod. because we want to have the same name sound that store in ActorDefinition, or elsewhere
/// we do not need the global sound definition
class Sound
{
public:
    explicit Sound(XmlElement const& soundElement);
    SoundID  GetSoundID() const;

    String  m_name     = "DEFAULT";
    String  m_filePath = "DEFAULT";
    SoundID m_id;
};
