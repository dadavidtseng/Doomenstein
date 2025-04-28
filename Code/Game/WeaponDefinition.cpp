//----------------------------------------------------------------------------------------------------
// WeaponDefinition.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/WeaponDefinition.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/HUD.hpp"
#include "Game/Sound.hpp"

//----------------------------------------------------------------------------------------------------
STATIC std::vector<WeaponDefinition*> WeaponDefinition::s_weaponDefinitions;

//----------------------------------------------------------------------------------------------------
WeaponDefinition::~WeaponDefinition()
{
    for (WeaponDefinition const* weaponDef : s_weaponDefinitions)
    {
        delete weaponDef;
    }

    s_weaponDefinitions.clear();
}

//----------------------------------------------------------------------------------------------------
bool WeaponDefinition::LoadFromXmlElement(XmlElement const* element)
{
    m_name = ParseXmlAttribute(*element, "name", "DEFAULT");
    if (m_name == "Pistol") m_refireTime = ParseXmlAttribute(*element, "refireTime", -1.f);
    if (m_name == "PlasmaRifle") m_refireTime = ParseXmlAttribute(*element, "refireTime", -1.f);
    if (m_name == "DemonMelee") m_refireTime = ParseXmlAttribute(*element, "refireTime", -1.f);
    m_rayCount        = ParseXmlAttribute(*element, "rayCount", -1);
    m_rayCone         = ParseXmlAttribute(*element, "rayCone", -1.f);
    m_rayRange        = ParseXmlAttribute(*element, "rayRange", -1.f);
    m_rayDamage       = ParseXmlAttribute(*element, "rayDamage", FloatRange::ZERO);
    m_rayImpulse      = ParseXmlAttribute(*element, "rayImpulse", -1.f);
    m_projectileCount = ParseXmlAttribute(*element, "projectileCount", -1);
    m_projectileCone  = ParseXmlAttribute(*element, "projectileCone", -1.f);
    m_projectileSpeed = ParseXmlAttribute(*element, "projectileSpeed", -1.f);
    m_projectileActor = ParseXmlAttribute(*element, "projectileActor", "DEFAULT");
    m_meleeCount      = ParseXmlAttribute(*element, "meleeCount", -1);
    m_meleeArc        = ParseXmlAttribute(*element, "meleeArc", -1.f);
    m_meleeRange      = ParseXmlAttribute(*element, "meleeRange", -1.f);
    m_meleeDamage     = ParseXmlAttribute(*element, "meleeDamage", FloatRange::ZERO);
    m_meleeRange      = ParseXmlAttribute(*element, "meleeRange", -1.f);
    m_meleeImpulse    = ParseXmlAttribute(*element, "meleeImpulse", -1.f);

    XmlElement const* hudElement = element->FirstChildElement("HUD");

    if (hudElement != nullptr)
    {
        m_hud = new HUD(*hudElement);
    }

    XmlElement const* soundElement = element->FirstChildElement("Sounds");

    if (soundElement != nullptr)
    {
        if (soundElement->ChildElementCount() > 0)
        {
            XmlElement const* soundChildElement = soundElement->FirstChildElement();

            while (soundChildElement != nullptr)
            {
                Sound sound = Sound(*soundChildElement);
                m_sounds.push_back(sound);
                soundChildElement = soundChildElement->NextSiblingElement();
            }
        }
    }

    return true;
}

//----------------------------------------------------------------------------------------------------
void WeaponDefinition::InitializeWeaponDefs(char const* path)
{
    XmlDocument     document;
    XmlResult const result = document.LoadFile(path);

    if (result != XmlResult::XML_SUCCESS)
    {
        ERROR_AND_DIE("Failed to load XML file")
    }

    XmlElement const* rootElement = document.RootElement();

    if (rootElement == nullptr)
    {
        ERROR_AND_DIE("XML file %s is missing a root element.")
    }

    XmlElement const* weaponDefinitionElement = rootElement->FirstChildElement();

    while (weaponDefinitionElement != nullptr)
    {
        String            elementName      = weaponDefinitionElement->Name();
        WeaponDefinition* weaponDefinition = new WeaponDefinition();

        if (weaponDefinition->LoadFromXmlElement(weaponDefinitionElement))
        {
            s_weaponDefinitions.push_back(weaponDefinition);
        }
        else
        {
            delete weaponDefinition;
            ERROR_AND_DIE("Failed to load actor definition")
        }

        weaponDefinitionElement = weaponDefinitionElement->NextSiblingElement();
    }
}

//----------------------------------------------------------------------------------------------------
WeaponDefinition * WeaponDefinition::GetDefByName(String const& name)
{
    for (WeaponDefinition * tileDef : s_weaponDefinitions)
    {
        if (tileDef->m_name == name)
        {
            return tileDef;
        }
    }

    return nullptr;
}

//----------------------------------------------------------------------------------------------------
Sound* WeaponDefinition::GetSoundByName( String const& soundName)
{
    for (Sound& sound : m_sounds)
    {
        if (sound.m_name == soundName)
            return &sound;
    }
    return nullptr;
}
