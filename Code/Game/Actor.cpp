//----------------------------------------------------------------------------------------------------
// Actor.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Actor.hpp"

#include "PlayerController.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"
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
}

//----------------------------------------------------------------------------------------------------
void Actor::Update(float const deltaSeconds)
{
    if (m_isDead)
    {
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
void Actor::Render() const
{
    if (m_definition->m_name == "SpawnPoint") return;
    if (!m_isVisible) return;

    VertexList_PCU verts;
    float const    eyeHeight         = m_definition->m_eyeHeight;
    Vec3 const     forwardNormal     = m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D().GetNormalized();
    Vec3 const     forwardNormalXY   = Vec3(forwardNormal.x, forwardNormal.y, 0.f).GetNormalized();
    Vec3 const     coneStartPosition = m_collisionCylinder.m_startPosition + Vec3(0.f, 0.f, eyeHeight) + forwardNormalXY * m_collisionCylinder.m_radius;

    if (m_definition->m_name != "PlasmaProjectile")
    {
        if (m_isDead)
        {
            AddVertsForCone3D(verts, coneStartPosition, coneStartPosition + forwardNormalXY * 0.1f, 0.1f, Interpolate(m_color, Rgba8::BLACK, 0.5f));
        }
        else
        {
            AddVertsForCone3D(verts, coneStartPosition, coneStartPosition + forwardNormalXY * 0.1f, 0.1f, m_color);
        }

        AddVertsForWireframeCone3D(verts, coneStartPosition, coneStartPosition + forwardNormalXY * 0.1f, 0.1f, 0.001f);
    }

    if (m_isDead)
    {
        AddVertsForCylinder3D(verts, m_collisionCylinder.m_startPosition, m_collisionCylinder.m_endPosition, m_collisionCylinder.m_radius, Interpolate(m_color, Rgba8::BLACK, 0.5f));
    }
    else
    {
        AddVertsForCylinder3D(verts, m_collisionCylinder.m_startPosition, m_collisionCylinder.m_endPosition, m_collisionCylinder.m_radius, m_color);
    }

    AddVertsForWireframeCylinder3D(verts, m_collisionCylinder.m_startPosition, m_collisionCylinder.m_endPosition, m_collisionCylinder.m_radius, 0.001f);

    g_theRenderer->SetModelConstants();
    g_theRenderer->SetBlendMode(eBlendMode::OPAQUE);
    g_theRenderer->SetRasterizerMode(eRasterizerMode::SOLID_CULL_BACK);
    g_theRenderer->SetSamplerMode(eSamplerMode::POINT_CLAMP);
    g_theRenderer->SetDepthMode(eDepthMode::READ_WRITE_LESS_EQUAL);
    g_theRenderer->BindTexture(nullptr);
    g_theRenderer->BindShader(g_theRenderer->CreateOrGetShaderFromFile("Default", eVertexType::VERTEX_PCU));

    g_theRenderer->DrawVertexArray(static_cast<int>(verts.size()), verts.data());
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

    if (!m_definition->m_isFlying)
    {
        m_position.z = 0.f;
    }

    m_acceleration = Vec3::ZERO;
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
