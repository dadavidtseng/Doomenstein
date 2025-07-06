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
    explicit Sound(XmlElement const& element);
    SoundID  GetSoundID() const;

    String  m_name     = "DEFAULT";     // Name of a specific sound to be played by this weapon. Possible values: Fire, played every time the weapon is fired.
    String  m_filePath = "DEFAULT";     // Audio file for this sound.
    SoundID m_id       ;
};
