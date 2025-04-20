#include "Sound.hpp"
#include "Game/GameCommon.hpp"

Sound::Sound(XmlElement const& soundElement)
{
    m_name     = ParseXmlAttribute(soundElement, "sound", m_name);
    m_filePath = ParseXmlAttribute(soundElement, "name", m_filePath);
    m_id       = g_theAudio->CreateOrGetSound(m_filePath);
}
