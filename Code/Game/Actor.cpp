//----------------------------------------------------------------------------------------------------
// Actor.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Actor.hpp"

#include "AnimationGroup.hpp"
#include "Game.hpp"
#include "PlayerController.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/AIController.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/Tile.hpp"
#include "Game/Weapon.hpp"
#include "Game/WeaponDefinition.hpp"

class SpriteAnimDefinition;
//----------------------------------------------------------------------------------------------------
Actor::Actor(SpawnInfo const& spawnInfo)
{
    m_definition = ActorDefinition::GetDefByName(spawnInfo.m_name);

    if (m_definition == nullptr)
    {
        ERROR_AND_DIE("Failed to find actor definition")
    }

    m_health      = m_definition->m_health;
    m_height      = m_definition->m_height;
    m_radius      = m_definition->m_radius;
    m_position    = spawnInfo.m_position;
    m_orientation = spawnInfo.m_orientation;
    m_velocity    = spawnInfo.m_velocity;

    for (String const& weapon : m_definition->m_inventory)
    {
        if (WeaponDefinition const* weaponDef = WeaponDefinition::GetDefByName(weapon))
        {
            m_weapons.push_back(new Weapon(this, weaponDef));
        }
    }

    if (!m_weapons.empty() && m_weapons[0] != nullptr)
    {
        m_currentWeapon = m_weapons[0];
    }

    if (spawnInfo.m_name == "Marine")
    {
        m_color = Rgba8::GREEN;
    }

    if (spawnInfo.m_name == "Demon")
    {
        m_color = Rgba8::RED;
    }

    if (spawnInfo.m_name == "PlasmaProjectile")
    {
        m_color = Rgba8::BLUE;
    }

    m_collisionCylinder = Cylinder3(m_position, m_position + Vec3(0.f, 0.f, m_height), m_radius);

    m_animationTimer = new Timer(0, g_theGame->m_gameClock); // Create timer
}

//----------------------------------------------------------------------------------------------------
void Actor::Update(float const deltaSeconds)
{
    if (m_isDead || m_definition->m_dieOnSpawn)
    {
        PlayAnimationByName("Death", true);
        m_isDead = true;
        m_dead += deltaSeconds;
    }

    if (m_dead > m_definition->m_corpseLifetime && m_definition->m_name != "SpawnPoint")
    {
        m_isGarbage = true;
    }

    if (!m_isDead)
    {
        UpdatePhysics(deltaSeconds);
    }

    if (m_aiController != nullptr && m_definition->m_aiEnabled && dynamic_cast<PlayerController*>(m_controller) == nullptr)
    {
        m_aiController->Update(deltaSeconds);
    }

    m_collisionCylinder.m_startPosition = m_position;
    m_collisionCylinder.m_endPosition   = m_position + Vec3(0.f, 0.f, m_height);
}

//----------------------------------------------------------------------------------------------------
// If visible, we will need vertexes and any other information necessary for rendering.
void Actor::Render(PlayerController const* toPlayer) const
{
    if (m_definition->m_name == "SpawnPoint") return;
    if (!m_isVisible) return;

    Mat44 localToWorldMat;
    Vec3  eyeHeight = Vec3(0.f, 0.f, m_definition->m_eyeHeight);
    // if (m_definition->m_billboardType == eBillboardType::NONE)localToWorldMat = GetModelToWorldTransform();
    // if (m_definition->m_billboardType == eBillboardType::WORLD_UP_FACING)localToWorldMat.Append(GetBillboardMatrix(eBillboardType::WORLD_UP_FACING, toPlayer->m_worldCamera->GetCameraToWorldTransform(), m_position));
    // if (m_definition->m_billboardType == eBillboardType::FULL_OPPOSING)localToWorldMat.Append(GetBillboardMatrix(eBillboardType::FULL_OPPOSING, toPlayer->m_worldCamera->GetCameraToWorldTransform(), m_position));
    // if (m_definition->m_billboardType == eBillboardType::WORLD_UP_OPPOSING)localToWorldMat.Append(GetBillboardMatrix(eBillboardType::WORLD_UP_OPPOSING, toPlayer->m_worldCamera->GetCameraToWorldTransform(), m_position));
    if (m_definition->m_billboardType == eBillboardType::NONE)
    {
        localToWorldMat = GetModelToWorldTransform();
    }
    else
    {
        if (m_definition->m_billboardType == eBillboardType::WORLD_UP_FACING)
        {
            // Mat44 cameraTransform = Mat44::MakeTranslation3D(toPlayer->m_position);
            // cameraTransform.Append(toPlayer->m_orientation.GetAsMatrix_IFwd_JLeft_KUp());
            // localToWorldMat = Mat44::MakeTranslation3D(m_position);
            localToWorldMat.Append(GetBillboardMatrix(eBillboardType::WORLD_UP_FACING, toPlayer->m_worldCamera->GetCameraToWorldTransform(), m_position));
        }
        else if (m_definition->m_billboardType == eBillboardType::FULL_OPPOSING)
        {
            // Mat44 cameraTransform = Mat44::MakeTranslation3D(toPlayer->m_position);
            // cameraTransform.Append(toPlayer->m_orientation.GetAsMatrix_IFwd_JLeft_KUp());
            localToWorldMat.Append(GetBillboardMatrix(eBillboardType::FULL_OPPOSING, toPlayer->m_worldCamera->GetCameraToWorldTransform(), m_position));
        }
        else if (m_definition->m_billboardType == eBillboardType::WORLD_UP_OPPOSING)
        {
            // Mat44 cameraTransform = Mat44::MakeTranslation3D(toPlayer->m_position);
            // cameraTransform.Append(toPlayer->m_orientation.GetAsMatrix_IFwd_JLeft_KUp());
            localToWorldMat.Append(GetBillboardMatrix(eBillboardType::WORLD_UP_OPPOSING, toPlayer->m_worldCamera->GetCameraToWorldTransform(), m_position + eyeHeight));
        }
        else
        {
            localToWorldMat = GetModelToWorldTransform();
        }
    }

    // verts.reserve(8192);
    // float const eyeHeight         = m_definition->m_eyeHeight;
    Vec3 const forwardNormal   = m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D().GetNormalized();
    Vec3 const forwardNormalXY = Vec3(forwardNormal.x, forwardNormal.y, 0.f).GetNormalized();
    // Vec3 const  coneStartPosition = m_collisionCylinder.m_startPosition + Vec3(0.f, 0.f, eyeHeight) + forwardNormalXY * m_collisionCylinder.m_radius;

    if (m_definition->m_name != "PlasmaProjectile")
    {
        if (m_isDead)
        {
            // AddVertsForCone3D(verts, coneStartPosition, coneStartPosition + forwardNormalXY * 0.1f, 0.1f, Interpolate(m_color, Rgba8::BLACK, 0.5f));
        }
        else
        {
            // AddVertsForCone3D(verts, coneStartPosition, coneStartPosition + forwardNormalXY * 0.1f, 0.1f, m_color);
        }

        // AddVertsForWireframeCone3D(verts, coneStartPosition, coneStartPosition + forwardNormalXY * 0.1f, 0.1f, 0.001f);
    }

    if (m_isDead)
    {
        // AddVertsForCylinder3D(verts, m_collisionCylinder.m_startPosition, m_collisionCylinder.m_endPosition, m_collisionCylinder.m_radius, Interpolate(m_color, Rgba8::BLACK, 0.5f));
    }
    else
    {
        // AddVertsForCylinder3D(verts, m_collisionCylinder.m_startPosition, m_collisionCylinder.m_endPosition, m_collisionCylinder.m_radius, m_color);
    }
    // AddVertsForWireframeCylinder3D(verts, m_collisionCylinder.m_startPosition, m_collisionCylinder.m_endPosition, m_collisionCylinder.m_radius, 0.001f);

    /// Get facing sprite UVs.
    Vec2 dirCameraToActorXY = Vec2(m_position.x - toPlayer->m_position.x, m_position.y - toPlayer->m_position.y).GetNormalized();
    Vec3 dirCameraToActor   = Vec3(dirCameraToActorXY.x, dirCameraToActorXY.y, 0.f).GetNormalized();
    Vec3 viewingDirection   = GetModelToWorldTransform().GetOrthonormalInverse().TransformVectorQuantity3D(dirCameraToActor);

    AnimationGroup const* animationGroup = m_currentPlayingAnimationGroup;
    if (animationGroup == nullptr && (int)m_definition->m_animationGroup.size() > 0) // We use the index 0 animation group
    {
        animationGroup = &m_definition->m_animationGroup[0];
    }

    SpriteAnimDefinition const* anim = &animationGroup->GetSpriteAnimation(viewingDirection);

    SpriteDefinition const spriteAtTime = anim->GetSpriteDefAtTime(m_animationTimer->GetElapsedTime() * 1); // TODO: Handle animation speed.
    AABB2                  uvAtTime     = spriteAtTime.GetUVs();

    Vec2 spriteOffSet = -m_definition->m_size * m_definition->m_pivot;
    Vec3 bottomLeft   = Vec3(0.f, spriteOffSet.x, spriteOffSet.y);
    Vec3 bottomRight  = bottomLeft + Vec3(0.f, m_definition->m_size.x, 0.f);
    Vec3 topLeft      = bottomLeft + Vec3(0.f, 0.f, m_definition->m_size.y);
    Vec3 topRight     = bottomRight + Vec3(0.f, 0.f, m_definition->m_size.y);

    bool const        bIsLit = m_definition->m_renderLit;
    VertexList_PCU    unlitVerts;
    VertexList_PCUTBN litVerts;

    if (bIsLit)
    {
        if (m_definition->m_renderRounded)
        {
            litVerts.reserve(12000);
            AddVertsForRoundedQuad3D(litVerts, topRight, bottomRight, bottomLeft, topLeft, Rgba8::WHITE, uvAtTime);
            g_theRenderer->SetModelConstants(localToWorldMat, Rgba8::WHITE);
            g_theRenderer->SetLightConstants(m_map->m_sunDirection, m_map->m_sunIntensity, m_map->m_ambientIntensity);
            g_theRenderer->SetBlendMode(eBlendMode::OPAQUE);
            g_theRenderer->SetDepthMode(eDepthMode::READ_WRITE_LESS_EQUAL);
            g_theRenderer->SetRasterizerMode(eRasterizerMode::SOLID_CULL_NONE);
            // g_theRenderer->SetRasterizerMode(eRasterizerMode::SOLID_CULL_BACK);
            g_theRenderer->SetSamplerMode(eSamplerMode::POINT_CLAMP);
            g_theRenderer->BindShader(m_definition->m_shader);
            g_theRenderer->BindTexture(&spriteAtTime.GetTexture());
            g_theRenderer->DrawVertexArray(litVerts);
            return;
        }

        litVerts.reserve(12000);
        AddVertsForQuad3D(litVerts, bottomLeft, bottomRight, topLeft, topRight, Rgba8::WHITE, uvAtTime);
        g_theRenderer->SetModelConstants(localToWorldMat, Rgba8::WHITE);
        g_theRenderer->SetLightConstants(m_map->m_sunDirection, m_map->m_sunIntensity, m_map->m_ambientIntensity);
        g_theRenderer->SetBlendMode(eBlendMode::OPAQUE);
        g_theRenderer->SetDepthMode(eDepthMode::READ_WRITE_LESS_EQUAL);
        g_theRenderer->SetRasterizerMode(eRasterizerMode::SOLID_CULL_NONE);
        // g_theRenderer->SetRasterizerMode(eRasterizerMode::SOLID_CULL_BACK);
        g_theRenderer->SetSamplerMode(eSamplerMode::POINT_CLAMP);
        g_theRenderer->BindShader(m_definition->m_shader);
        g_theRenderer->BindTexture(&spriteAtTime.GetTexture());
        g_theRenderer->DrawVertexArray(litVerts);
        return;
    }
    else
    {
        if (m_definition->m_renderRounded)
        {
            // unlitVerts.reserve(12000);
            // AddVertsForRoundedQuad3D(litVerts, topRight, bottomRight, bottomLeft, topLeft, Rgba8::WHITE, uvAtTime);
            // g_theRenderer->SetModelConstants(localToWorldMat, Rgba8::WHITE);
            // g_theRenderer->SetBlendMode(eBlendMode::OPAQUE);
            // g_theRenderer->SetDepthMode(eDepthMode::READ_WRITE_LESS_EQUAL);
            // g_theRenderer->SetRasterizerMode(eRasterizerMode::SOLID_CULL_NONE);
            // // g_theRenderer->SetRasterizerMode(eRasterizerMode::SOLID_CULL_BACK);
            // g_theRenderer->SetSamplerMode(eSamplerMode::POINT_CLAMP);
            // g_theRenderer->BindShader(m_definition->m_shader);
            // g_theRenderer->BindTexture(&spriteAtTime.GetTexture());
            // g_theRenderer->DrawVertexArray(unlitVerts);
            return;
        }
        else
        {
            unlitVerts.reserve(12000);
            AddVertsForQuad3D(unlitVerts, bottomLeft, bottomRight, topLeft, topRight, Rgba8::WHITE, uvAtTime);
            g_theRenderer->SetModelConstants(localToWorldMat, Rgba8::WHITE);
            g_theRenderer->SetBlendMode(eBlendMode::OPAQUE);
            g_theRenderer->SetDepthMode(eDepthMode::READ_WRITE_LESS_EQUAL);
            g_theRenderer->SetRasterizerMode(eRasterizerMode::SOLID_CULL_NONE);
            // g_theRenderer->SetRasterizerMode(eRasterizerMode::SOLID_CULL_BACK);
            g_theRenderer->SetSamplerMode(eSamplerMode::POINT_CLAMP);
            g_theRenderer->BindShader(m_definition->m_shader);
            g_theRenderer->BindTexture(&spriteAtTime.GetTexture());
            g_theRenderer->DrawVertexArray(unlitVerts);
            return;
        }
    }


    // bool const bIsLit = m_definition->m_renderLit;
    // bool const renderRounded = m_definition->m_renderRounded;
    //
    // VertexList_PCUTBN litVerts;
    // VertexList_PCU    unlitVerts;
    //
    // Rgba8 const color = Rgba8::WHITE;
    //
    // if (bIsLit)
    // {
    //     litVerts.reserve(12000);
    //
    //     if (renderRounded)
    //     {
    //         AddVertsForRoundedQuad3D(litVerts, topRight, bottomRight, bottomLeft, topLeft, color, uvAtTime);
    //     }
    //     else
    //     {
    //         AddVertsForQuad3D(unlitVerts, bottomLeft, bottomRight, topLeft, topRight, color, uvAtTime);
    //     }
    //
    //     g_theRenderer->SetModelConstants(localToWorldMat, color);
    //     g_theRenderer->SetLightConstants(m_map->m_sunDirection, m_map->m_sunIntensity, m_map->m_ambientIntensity);
    //     g_theRenderer->SetBlendMode(eBlendMode::OPAQUE);
    //     g_theRenderer->SetDepthMode(eDepthMode::READ_WRITE_LESS_EQUAL);
    //     g_theRenderer->SetRasterizerMode(eRasterizerMode::SOLID_CULL_BACK);
    //     g_theRenderer->SetSamplerMode(eSamplerMode::POINT_CLAMP);
    //     g_theRenderer->BindShader(m_definition->m_shader);
    //     g_theRenderer->BindTexture(&spriteAtTime.GetTexture());
    //     g_theRenderer->DrawVertexArray(renderRounded ? litVerts : litVerts); // always litVerts in lit path
    //     return;
    // }
    // else
    // {
    //     unlitVerts.reserve(12000);
    //
    //     if (renderRounded)
    //     {
    //         AddVertsForRoundedQuad3D(unlitVerts, topRight, bottomRight, bottomLeft, topLeft, color, uvAtTime);
    //     }
    //     else
    //     {
    //         AddVertsForQuad3D(unlitVerts, bottomLeft, bottomRight, topLeft, topRight, color, uvAtTime);
    //     }
    //
    //     g_theRenderer->SetModelConstants(localToWorldMat, color);
    //     g_theRenderer->SetBlendMode(eBlendMode::OPAQUE);
    //     g_theRenderer->SetDepthMode(eDepthMode::READ_WRITE_LESS_EQUAL);
    //     g_theRenderer->SetRasterizerMode(eRasterizerMode::SOLID_CULL_BACK);
    //     g_theRenderer->SetSamplerMode(eSamplerMode::POINT_CLAMP);
    //     g_theRenderer->BindShader(m_definition->m_shader);
    //     g_theRenderer->BindTexture(&spriteAtTime.GetTexture());
    //     g_theRenderer->DrawVertexArray(unlitVerts);
    //     return;
    // }
}

//----------------------------------------------------------------------------------------------------
Mat44 Actor::GetModelToWorldTransform() const
{
    Mat44 m2w;

    m2w.SetTranslation3D(m_position);
    m2w.Append(m_orientation.GetAsMatrix_IFwd_JLeft_KUp());

    return m2w;
}

void Actor::UpdatePhysics(float const deltaSeconds)
{
    float const dragValue = m_definition->m_drag;
    Vec3 const  dragForce = -m_velocity * dragValue;
    AddForce(dragForce);

    m_velocity += m_acceleration * deltaSeconds;
    m_position += m_velocity * deltaSeconds;

    if (!m_definition->m_flying)
    {
        if (m_definition->m_name != "BulletHit" &&
            m_definition->m_name != "BloodSplatter")
        {
            m_position.z = 0.f;
        }
    }

    m_acceleration = Vec3::ZERO;
}

void Actor::UpdateAnimation(float const deltaSeconds)
{
    UNUSED(deltaSeconds)
    if (!m_currentPlayingAnimationGroup) return;
    if (m_animationTimer->GetElapsedTime() > m_currentPlayingAnimationGroup->GetAnimationLength())
    {
        m_currentPlayingAnimationGroup = nullptr;
        m_animationTimer->Stop();
    }
    if (m_definition->m_runSpeed != 0.f) // Zero safe check
        m_animationTimerSpeedMultiplier = m_velocity.GetLength() / m_definition->m_runSpeed;
}

void Actor::Damage(int const          damage,
                   ActorHandle const& other)
{
    m_health -= damage;

    if (m_health < 0)
    {
        m_isDead = true;
    }

    if (m_aiController != nullptr)
    {
        m_aiController->DamagedBy(other);
    }
}

void Actor::AddForce(Vec3 const& force)
{
    m_acceleration += force;
}

void Actor::AddImpulse(Vec3 const& impulse)
{
    m_velocity += impulse;
}

void Actor::MoveInDirection(Vec3 const& direction,
                            float const speed)
{
    Vec3 const  directionNormal = direction.GetNormalized();
    float const dragValue       = m_definition->m_drag;
    Vec3 const  force           = directionNormal * speed * dragValue;
    AddForce(force);
}

void Actor::TurnInDirection(EulerAngles const& direction)
{
    m_orientation = direction;
}

//----------------------------------------------------------------------------------------------------
void Actor::OnPossessed(Controller* controller)
{
    m_controller = controller;
    m_isVisible  = false;
}

//----------------------------------------------------------------------------------------------------
// a. Actors remember who their controller is.
// b. If the player possesses an actor with an AI controller, the AI controller is saved and then restored if the player unpossesses the actor.
void Actor::OnUnpossessed()
{
    m_controller = nullptr;
    m_isVisible  = true;

    if (m_aiController == nullptr) return;
    m_controller = m_aiController;
}

void Actor::OnCollisionEnterWithActor(Actor* other)
{
    if (m_isDead || other->m_isDead) return;
    if (m_owner && other->m_owner) return;
    if (m_owner && other->m_definition->m_name == "Marine") return;
    if (this == other) return;

    Vec2 positionXY       = Vec2(m_position.x, m_position.y);
    Vec2 otherPositionXY  = Vec2(other->m_position.x, other->m_position.y);
    Vec2 actorAPositionXY = Vec2(m_position.x, m_position.y);

    if (DoDiscsOverlap2D(positionXY, m_radius, otherPositionXY, other->m_radius))
    {
        if (m_definition->m_name == "PlasmaProjectile" && other->m_definition->m_name == "Demon")
        {
            int randomDamage = (int)g_theRNG->RollRandomFloatInRange(m_definition->m_damageOnCollide.m_min, m_definition->m_damageOnCollide.m_max);
            other->Damage(randomDamage, m_owner->m_handle);
            Vec3 forward, left, right;
            m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, right);
            other->AddImpulse(forward);
            m_isDead = true;
        }
    }

    Vec2        actorBPositionXY = Vec2(other->m_position.x, other->m_position.y);
    float const actorARadius     = m_radius;
    float const actorBRadius     = other->m_radius;

    // 5. Push movable actor out of immovable actor.
    PushDiscsOutOfEachOther2D(actorAPositionXY, actorARadius, actorBPositionXY, actorBRadius);

    // 6. Update actors' position.
    m_position.x        = actorAPositionXY.x;
    m_position.y        = actorAPositionXY.y;
    other->m_position.x = actorBPositionXY.x;
    other->m_position.y = actorBPositionXY.y;
}

void Actor::OnCollisionEnterWithMap(IntVec2 const& tileCoords)
{
    // TODO: Swap check method for Sprinting if needed (PushCapsuleOutOfAABB2D/DoCapsuleAndAABB2Overlap2D)

    AABB3 const aabb3Box = m_map->GetTile(tileCoords.x, tileCoords.y)->m_bounds;
    AABB2 const aabb2Box = AABB2(Vec2(aabb3Box.m_mins.x, aabb3Box.m_mins.y), Vec2(aabb3Box.m_maxs.x, aabb3Box.m_maxs.y));

    Vec2 actorPositionXY = Vec2(m_position.x, m_position.y);

    bool const isPushed = PushDiscOutOfAABB2D(actorPositionXY, m_radius, aabb2Box);

    if (isPushed && m_definition->m_dieOnCollide)
    {
        m_isDead = true;
    }

    m_position.x = actorPositionXY.x;
    m_position.y = actorPositionXY.y;
}

void Actor::OnCollisionEnterWithMap(AABB3 const& bounds)
{
    float zCylinderMaxZ = m_position.z + m_height;
    float zCylinderMinZ = m_position.z;

    if (zCylinderMaxZ > bounds.m_maxs.z || zCylinderMinZ < bounds.m_mins.z)
    {
        if (m_definition->m_dieOnCollide) m_isDead = true;
    }

    if (zCylinderMaxZ > bounds.m_maxs.z)
    {
        zCylinderMaxZ = bounds.m_maxs.z;
        m_position.z  = zCylinderMaxZ - m_height;
    }

    if (zCylinderMinZ < bounds.m_mins.z)
    {
        zCylinderMinZ = bounds.m_mins.z;
        m_position.z  = zCylinderMinZ;
    }
}

void Actor::Attack() const
{
    if (m_currentWeapon == nullptr) return;

    m_currentWeapon->Fire();
}

void Actor::SwitchInventory(unsigned int const index)
{
    if (index < m_weapons.size())
    {
        if (m_currentWeapon != m_weapons[index])
        {
            m_currentWeapon = m_weapons[index];
        }
    }
}

Vec3 Actor::GetActorEyePosition() const
{
    return m_position + Vec3(0.f, 0.f, m_definition->m_eyeHeight);
}

AnimationGroup* Actor::PlayAnimationByName(String const& animationName,
                                           bool const    force)
{
    AnimationGroup* foundedGroup = m_definition->GetAnimationGroupByName(animationName);

    if (foundedGroup)
    {
        if (foundedGroup == m_currentPlayingAnimationGroup)
        {
            return foundedGroup;
        }
        else
        {
            /// We want to replace to new animation, force update it whether or not it finished
            if (force)
            {
                m_currentPlayingAnimationGroup = foundedGroup;
                m_animationTimer->Start();
                return foundedGroup;
            }
            else
            {
                if (m_currentPlayingAnimationGroup)
                {
                    bool isCurrentAnimFinished = m_animationTimer->GetElapsedTime() >= m_currentPlayingAnimationGroup->GetAnimationLength();
                    if (isCurrentAnimFinished)
                    {
                        m_currentPlayingAnimationGroup = foundedGroup;
                        m_animationTimer->Start();
                        return foundedGroup;
                    }
                }
                else
                {
                    m_currentPlayingAnimationGroup = foundedGroup;
                    m_animationTimer->Start();
                    return foundedGroup;
                }
            }
        }
    }
    return nullptr;
}
