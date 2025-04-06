//----------------------------------------------------------------------------------------------------
// Controller.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Controller.hpp"

#include "Game/Actor.hpp"
#include "Game/Map.hpp"

//----------------------------------------------------------------------------------------------------
Controller::Controller(Map* owner)
    : m_map(owner)
{
}

//----------------------------------------------------------------------------------------------------
// Unpossess any currently possessed actor and possess a new one.
// Notify each actor so it can check for restoring AI controllers or handle other change of possession logic.
void Controller::Possess(ActorHandle const& actorHandle)
{
    Actor* currentPossessedActor = m_map->GetActorByHandle(m_actorHandle);

    if (currentPossessedActor != nullptr &&
        currentPossessedActor->m_handle.IsValid())
    {
        currentPossessedActor->OnUnpossessed();
    }

    Actor* newPossessedActor = m_map->GetActorByHandle(actorHandle);

    if (newPossessedActor != nullptr &&
        newPossessedActor->m_handle.IsValid())
    {
        newPossessedActor->OnPossessed(this);
    }

    m_actorHandle = actorHandle;
}

//----------------------------------------------------------------------------------------------------
// Returns the currently possessed actor or null if no actor is possessed.
Actor* Controller::GetActor()
{
    return m_map->GetActorByHandle(m_actorHandle);
}
