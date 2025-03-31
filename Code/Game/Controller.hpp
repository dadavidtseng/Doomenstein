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
    explicit Controller(Map* owner);
    virtual  ~Controller() = default;

    virtual void         Possess(ActorHandle& actorHandle);
    virtual Actor const* GetActor();

protected:
    // The reference to controller's actor is by actor handle.
    ActorHandle m_actorHandle;           // Handle of our currently possessed actor or INVALID if no actor is possessed.
    Map*        m_map = nullptr;    // Reference to the current map for purposes of dereferencing actor handles.
};
