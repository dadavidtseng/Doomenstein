//----------------------------------------------------------------------------------------------------
// AIController.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include "Game/Controller.hpp"

//----------------------------------------------------------------------------------------------------
// AI controllers should be constructed by the actor when the actor is spawned and immediately possess that actor.
class AIController final : public Controller
{
public:
    // Construction / Destruction
    explicit AIController(Map* map);

    void Update(float deltaSeconds) override;
    void DamagedBy(ActorHandle const& attacker);

    ActorHandle m_targetActorHandle;
};
