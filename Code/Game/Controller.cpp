//----------------------------------------------------------------------------------------------------
// Controller.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Controller.hpp"

#include "Actor.hpp"
#include "Map.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//----------------------------------------------------------------------------------------------------
Controller::Controller(Map* owner)
    : m_map(owner)
{
}

//----------------------------------------------------------------------------------------------------
// Unpossess any currently possessed actor and possess a new one.
// Notify each actor so it can check for restoring AI controllers or handle other change of possession logic.
void Controller::Possess(ActorHandle& actorHandle)
{
    Actor* currentPossessActor = m_map->GetActorByHandle(m_actorHandle);

    if (currentPossessActor && currentPossessActor->m_handle.IsValid())
    {
        currentPossessActor->OnUnpossessed();
    }

    Actor* newPossessActor = m_map->GetActorByHandle(actorHandle);

    if (newPossessActor && newPossessActor->m_handle.IsValid())
    {
        newPossessActor->OnPossessed(this);
    }

    m_actorHandle = actorHandle;

    DebuggerPrintf("%d\n",m_actorHandle.GetIndex());
}

//----------------------------------------------------------------------------------------------------
// Returns the currently possessed actor or null if no actor is possessed.
Actor const* Controller::GetActor()
{
    return m_map->GetActorByHandle(m_actorHandle);
}
