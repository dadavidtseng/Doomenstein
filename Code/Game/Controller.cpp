//----------------------------------------------------------------------------------------------------
// Controller.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Controller.hpp"

#include "Map.hpp"

//----------------------------------------------------------------------------------------------------
Controller::Controller(Map* owner)
    : m_map(owner)
{
}

//----------------------------------------------------------------------------------------------------
// Unpossess any currently possessed actor and possess a new one.
// Notify each actor so it can check for restoring AI controllers or handle other change of possession logic.
void Controller::Possess()
{
}

//----------------------------------------------------------------------------------------------------
// Returns the currently possessed actor or null if no actor is possessed.
Actor const* Controller::GetActor()
{
    return m_map->GetActorByHandle(m_handle);
}
