//----------------------------------------------------------------------------------------------------
// ActorDefinition.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/ActorDefinition.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//----------------------------------------------------------------------------------------------------
std::vector<ActorDefinition*> ActorDefinition::s_actorDefinitions;

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
    m_name           = ParseXmlAttribute(*element, "name", "Unnamed");
    m_faction        = ParseXmlAttribute(*element, "faction", "Unnamed");
    m_health         = ParseXmlAttribute(*element, "health", -1);
    m_canBePossessed = ParseXmlAttribute(*element, "canBePossessed", false);
    m_corpseLifetime = ParseXmlAttribute(*element, "corpseLifetime", -1.f);
    m_isVisible      = ParseXmlAttribute(*element, "isVisible", false);

    XmlElement const* collisionElement = element->FirstChildElement("Collision");

    if (collisionElement != nullptr)
    {
        m_radius             = ParseXmlAttribute(*collisionElement, "radius", -1.f);
        m_height             = ParseXmlAttribute(*collisionElement, "height", -1.f);
        m_collidesWithWorld  = ParseXmlAttribute(*collisionElement, "collidesWithWorld", false);
        m_collidesWithActors = ParseXmlAttribute(*collisionElement, "collidesWithActors", false);
    }

    XmlElement const* physicsElement = element->FirstChildElement("Physics");

    if (physicsElement != nullptr)
    {
        m_isSimulated = ParseXmlAttribute(*physicsElement, "simulated", false);
        m_walkSpeed   = ParseXmlAttribute(*physicsElement, "walkSpeed", -1.f);
        m_runSpeed    = ParseXmlAttribute(*physicsElement, "runSpeed", -1.f);
        m_turnSpeed   = ParseXmlAttribute(*physicsElement, "turnSpeed", -1.f);
        m_drag        = ParseXmlAttribute(*physicsElement, "drag", -1.f);
    }

    XmlElement const* cameraElement = element->FirstChildElement("Camera");

    if (cameraElement != nullptr)
    {
        m_eyeHeight = ParseXmlAttribute(*cameraElement, "eyeHeight", -1.f);
        m_cameraFOV = ParseXmlAttribute(*cameraElement, "cameraFOV", -1.f);
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
        ERROR_AND_DIE("Failed to load XML file: %s\n", path)
    }

    XmlElement const* rootElement = document.RootElement();

    if (rootElement == nullptr)
    {
        ERROR_AND_DIE("XML file %s is missing a root element.\n", path)
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
            delete actorDefinition;
            ERROR_AND_DIE("Failed to load actor definition: %s\n", elementName.c_str())
        }

        actorDefinitionElement = actorDefinitionElement->NextSiblingElement();
    }
}

ActorDefinition const* ActorDefinition::GetDefByName(String const& name)
{
    for (ActorDefinition const* actorDef : s_actorDefinitions)
    {
        if (actorDef->m_name == name)
        {
            return actorDef;
        }
    }

    return nullptr;
}
