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
bool ActorDefinition::LoadFromXmlElement(XmlElement const& element)
{
    m_name   = ParseXmlAttribute(element, "name", "Unnamed");
    m_health = ParseXmlAttribute(element, "health", 0);
    if (m_name == "Unnamed") return false;

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

        if (actorDefinition->LoadFromXmlElement(*actorDefinitionElement))
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
