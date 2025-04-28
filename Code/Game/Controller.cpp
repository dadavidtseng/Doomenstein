//----------------------------------------------------------------------------------------------------
// Controller.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Controller.hpp"

#include "Game.hpp"
#include "GameCommon.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Window.hpp"
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

//----------------------------------------------------------------------------------------------------
void Controller::SetControllerIndex(int const index)
{
    m_index = index;
}

//----------------------------------------------------------------------------------------------------
AABB2 Controller::SetViewport(AABB2 const& viewPort)
{
    m_viewport = viewPort;
    m_worldCamera->SetNormalizedViewport(viewPort);
    m_viewCamera->SetNormalizedViewport(viewPort);
    m_screenViewport = m_viewCamera->GetViewPortUnnormalized(Vec2(Window::s_mainWindow->GetClientDimensions().x, Window::s_mainWindow->GetClientDimensions().y));

    return viewPort;
}

//----------------------------------------------------------------------------------------------------
int Controller::GetControllerIndex() const
{
    return m_index;
}
