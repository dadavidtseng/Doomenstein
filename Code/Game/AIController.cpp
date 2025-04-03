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

void AIController::DamagedBy()
{
}
