//----------------------------------------------------------------------------------------------------
// AI.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include "Game/Controller.hpp"

//----------------------------------------------------------------------------------------------------
// AI controllers should be constructed by the actor when the actor is spawned and immediately possess that actor.
class AIController : public Controller
{
    void DamagedBy();
    void Update();

    ActorHandle m_targetActorHandle;
};
