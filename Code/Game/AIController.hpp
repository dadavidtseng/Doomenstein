//----------------------------------------------------------------------------------------------------
// AIController.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include "Game/Controller.hpp"

//----------------------------------------------------------------------------------------------------
// AI controllers should be constructed by the actor when the actor is spawned and immediately possess that actor.
class AIController : public Controller
{
public:
    explicit AIController(Map* map);
    ~AIController() override;

    void Update(float deltaSeconds) override;
    void DamagedBy(ActorHandle const& attacker);

    ActorHandle m_targetActorHandle;
};
