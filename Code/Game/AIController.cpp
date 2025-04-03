//----------------------------------------------------------------------------------------------------
// AI.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/AIController.hpp"

#include "Map.hpp"

//----------------------------------------------------------------------------------------------------
AIController::AIController(Map* map)
    : Controller(map)
{
}

AIController::~AIController()
{
}

void AIController::Update(float deltaSeconds)
{
    m_map->GetClosestVisibleEnemy(m_map->GetActorByHandle(m_actorHandle));
}

//----------------------------------------------------------------------------------------------------
// Notification that the AI actor was damaged so this AI can target them.
void AIController::DamagedBy(ActorHandle const& attacker)
{
    m_targetActorHandle = attacker;
}
