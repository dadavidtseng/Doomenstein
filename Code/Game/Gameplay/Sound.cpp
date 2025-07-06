//----------------------------------------------------------------------------------------------------
// Sound.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Gameplay/Sound.hpp"

#include "Game/Framework/GameCommon.hpp"

//----------------------------------------------------------------------------------------------------
Sound::Sound(XmlElement const& element)
{
    m_name     = ParseXmlAttribute(element, "sound", m_name);
    m_filePath = ParseXmlAttribute(element, "name", m_filePath);
    m_id       = g_theAudio->CreateOrGetSound(m_filePath, eAudioSystemSoundDimension::Sound3D);
}

//----------------------------------------------------------------------------------------------------
SoundID Sound::GetSoundID() const
{
    return m_id;
}
