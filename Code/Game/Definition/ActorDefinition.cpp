//----------------------------------------------------------------------------------------------------
// ActorDefinition.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Definition/ActorDefinition.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Framework/AnimationGroup.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Gameplay/Sound.hpp"

//----------------------------------------------------------------------------------------------------
STATIC std::vector<ActorDefinition*> ActorDefinition::s_actorDefinitions;

//----------------------------------------------------------------------------------------------------
ActorDefinition::~ActorDefinition()
{
    for (ActorDefinition const* actorDef : s_actorDefinitions)
    {
        delete actorDef;
    }

    s_actorDefinitions.clear();
}

//----------------------------------------------------------------------------------------------------
bool ActorDefinition::LoadFromXmlElement(XmlElement const* element)
{
    m_name           = ParseXmlAttribute(*element, "name", "DEFAULT");
    m_faction        = ParseXmlAttribute(*element, "faction", "NEUTRAL");
    m_health         = ParseXmlAttribute(*element, "health", -1);
    m_canBePossessed = ParseXmlAttribute(*element, "canBePossessed", false);
    m_corpseLifetime = ParseXmlAttribute(*element, "corpseLifetime", -1.f);
    m_isVisible      = ParseXmlAttribute(*element, "visible", false);
    m_dieOnSpawn     = ParseXmlAttribute(*element, "dieOnSpawn", false);

    XmlElement const* collisionElement = element->FirstChildElement("Collision");

    if (collisionElement != nullptr)
    {
        m_radius             = ParseXmlAttribute(*collisionElement, "radius", -1.f);
        m_height             = ParseXmlAttribute(*collisionElement, "height", -1.f);
        m_impulseOnCollide   = ParseXmlAttribute(*collisionElement, "impulseOnCollide", -1.f);
        m_damageOnCollide    = ParseXmlAttribute(*collisionElement, "damageOnCollide", FloatRange::ZERO);
        m_collidesWithWorld  = ParseXmlAttribute(*collisionElement, "collidesWithWorld", false);
        m_collidesWithActors = ParseXmlAttribute(*collisionElement, "collidesWithActors", false);
        m_dieOnCollide       = ParseXmlAttribute(*collisionElement, "dieOnCollide", false);
    }

    XmlElement const* physicsElement = element->FirstChildElement("Physics");

    if (physicsElement != nullptr)
    {
        m_simulated = ParseXmlAttribute(*physicsElement, "simulated", false);
        m_flying    = ParseXmlAttribute(*physicsElement, "flying", false);
        m_walkSpeed = ParseXmlAttribute(*physicsElement, "walkSpeed", -1.f);
        m_runSpeed  = ParseXmlAttribute(*physicsElement, "runSpeed", -1.f);
        m_turnSpeed = ParseXmlAttribute(*physicsElement, "turnSpeed", -1.f);
        m_drag      = ParseXmlAttribute(*physicsElement, "drag", -1.f);
    }

    XmlElement const* cameraElement = element->FirstChildElement("Camera");

    if (cameraElement != nullptr)
    {
        m_eyeHeight = ParseXmlAttribute(*cameraElement, "eyeHeight", -1.f);
        m_cameraFOV = ParseXmlAttribute(*cameraElement, "cameraFOV", -1.f);
    }

    XmlElement const* aiElement = element->FirstChildElement("AI");

    if (aiElement != nullptr)
    {
        m_aiEnabled   = ParseXmlAttribute(*aiElement, "aiEnabled", false);
        m_sightRadius = ParseXmlAttribute(*aiElement, "sightRadius", -1.f);
        m_sightAngle  = ParseXmlAttribute(*aiElement, "sightAngle", -1.f);
    }

    XmlElement const* visualElement = element->FirstChildElement("Visuals");

    if (visualElement != nullptr)
    {
        m_size                     = ParseXmlAttribute(*visualElement, "size", Vec2::ZERO);
        m_pivot                    = ParseXmlAttribute(*visualElement, "pivot", Vec2::ZERO);
        String const billboardType = ParseXmlAttribute(*visualElement, "billboardType", "DEFAULT");
        if (billboardType == "FullFacing") m_billboardType = eBillboardType::FULL_FACING;
        if (billboardType == "FullOpposing") m_billboardType = eBillboardType::FULL_OPPOSING;
        if (billboardType == "WorldUpFacing") m_billboardType = eBillboardType::WORLD_UP_FACING;
        if (billboardType == "WorldUpOpposing") m_billboardType = eBillboardType::WORLD_UP_OPPOSING;
        m_renderLit                       = ParseXmlAttribute(*visualElement, "renderLit", false);
        m_renderRounded                   = ParseXmlAttribute(*visualElement, "renderRounded", false);
        m_cellCount                       = ParseXmlAttribute(*visualElement, "cellCount", IntVec2::ZERO);
        String const shaderPath           = ParseXmlAttribute(*visualElement, "shader", "DEFAULT");
        m_shader                          = g_theRenderer->CreateOrGetShaderFromFile(shaderPath.c_str(), eVertexType::VERTEX_PCUTBN);
        String const   spriteSheetPath    = ParseXmlAttribute(*visualElement, "spriteSheet", "DEFAULT");
        Texture const* spriteSheetTexture = g_theRenderer->CreateOrGetTextureFromFile(spriteSheetPath.c_str());
        m_spriteSheet                     = new SpriteSheet(*spriteSheetTexture, m_cellCount);

        if (visualElement->ChildElementCount() > 0)
        {
            XmlElement const* visualChildElement = visualElement->FirstChildElement();

            while (visualChildElement != nullptr)
            {
                AnimationGroup animationGroup = AnimationGroup(*visualChildElement, *m_spriteSheet);
                m_animationGroup.push_back(animationGroup);
                visualChildElement = visualChildElement->NextSiblingElement();
            }
        }
    }

    XmlElement const* soundsElement = element->FirstChildElement("Sounds");

    if (soundsElement != nullptr)
    {
        XmlElement const* soundElement = soundsElement->FirstChildElement("Sound");

        while (soundElement != nullptr)
        {
            Sound sound = Sound(*soundElement);

            m_sounds.push_back(sound);

            soundElement = soundElement->NextSiblingElement("Sound");
        }
    }

    XmlElement const* inventoryElement = element->FirstChildElement("Inventory");

    if (inventoryElement != nullptr)
    {
        XmlElement const* weaponElement = inventoryElement->FirstChildElement("Weapon");

        while (weaponElement != nullptr)
        {
            String weaponName;

            weaponName = ParseXmlAttribute(*weaponElement, "name", "DEFAULT");

            m_inventory.push_back(weaponName);

            weaponElement = weaponElement->NextSiblingElement("Weapon");
        }
    }

    return true;
}

//----------------------------------------------------------------------------------------------------
STATIC void ActorDefinition::InitializeActorDefs(char const* path)
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

    XmlElement const* actorDefinitionElement = rootElement->FirstChildElement();

    while (actorDefinitionElement != nullptr)
    {
        String           elementName     = actorDefinitionElement->Name();
        ActorDefinition* actorDefinition = new ActorDefinition();

        if (actorDefinition->LoadFromXmlElement(actorDefinitionElement))
        {
            s_actorDefinitions.push_back(actorDefinition);
        }
        else
        {
            delete &actorDefinition;
            ERROR_AND_DIE("Failed to load actor definition")
        }

        actorDefinitionElement = actorDefinitionElement->NextSiblingElement();
    }
}

//----------------------------------------------------------------------------------------------------
ActorDefinition* ActorDefinition::GetDefByName(String const& name)
{
    for (ActorDefinition* actorDef : s_actorDefinitions)
    {
        if (actorDef->m_name == name)
        {
            return actorDef;
        }
    }

    return nullptr;
}

//----------------------------------------------------------------------------------------------------
AnimationGroup* ActorDefinition::GetAnimationGroupByName(String const& name)
{
    for (AnimationGroup& animGroup : m_animationGroup)
    {
        if (animGroup.GetName() == name) return &animGroup;
    }
    return nullptr;
}

//----------------------------------------------------------------------------------------------------
Sound* ActorDefinition::GetSoundByName(String const& name)
{
    for (Sound& sound : m_sounds)
    {
        if (sound.m_name == name) return &sound;
    }
    return nullptr;
}
