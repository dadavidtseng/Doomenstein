//----------------------------------------------------------------------------------------------------
// Actor.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include <vector>

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Cylinder3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"

//-Forward-Declaration--------------------------------------------------------------------------------
class AIController;
class Map;
class PlayerController;
class Weapon;
struct ActorDefinition;

//----------------------------------------------------------------------------------------------------
enum class eActorFaction : int8_t
{
    INVALID = -1,
    MARINE,
    DEMON,
    BULLET,
    COUNT
};

//----------------------------------------------------------------------------------------------------
class Actor
{
public:
    // Actor(Vec3 const& position, EulerAngles const& orientation, float radius, float height, bool isMovable, Rgba8 const& color);
    Actor(ActorDefinition const& actorDefinition);

    void  Update();
    void  UpdatePosition();
    void  Render() const;
    Mat44 GetModelToWorldTransform() const;
    void  UpdatePhysics();
    void  Damage();
    void  AddForce();
    void  AddImpulse();
    void  OnCollide();
    void  OnPossessed();
    void  OnUnpossessed();
    void  MoveInDirection();
    void  TurnInDirection();
    void  Attack();
    void  EquipWeapon();

    Vec3        m_position        = Vec3::ZERO;
    Vec3        m_velocity        = Vec3::ZERO;
    EulerAngles m_orientation     = EulerAngles::ZERO;
    EulerAngles m_angularVelocity = EulerAngles::ZERO;
    float       m_radius          = 0.f;
    float       m_height          = 0.f;
    bool        m_isMovable       = false;
    Rgba8       m_color           = Rgba8::WHITE;
    Cylinder3   m_cylinder;

    ActorDefinition const* m_actorDefinition = nullptr;
    bool                   m_isDead          = false;
    int                    m_health          = -1;
    Map*                   m_map             = nullptr;
    std::vector<Weapon*>   m_weapons;
    PlayerController*      m_playerController = nullptr;
    AIController*          m_aiController     = nullptr;
};
