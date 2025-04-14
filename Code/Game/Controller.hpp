//----------------------------------------------------------------------------------------------------
// Controller.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include "Game/ActorHandle.hpp"

//-Forward-Declaration--------------------------------------------------------------------------------
class Actor;
class Map;

//----------------------------------------------------------------------------------------------------
// Controllers may possess, and control, an actor.
class Controller
{
public:
    // Construction / Destruction
    explicit     Controller(Map* owner);
    virtual      ~Controller() = default;

    virtual void Update(float deltaSeconds) = 0;

    virtual void   Possess(ActorHandle const& actorHandle);
    virtual Actor* GetActor();
    // The reference to controller's actor is by actor handle.
    ActorHandle m_actorHandle;           // Handle of our currently possessed actor or INVALID if no actor is possessed.
    Map*        m_map = nullptr;    // Reference to the current map for purposes of dereferencing actor handles.
};
