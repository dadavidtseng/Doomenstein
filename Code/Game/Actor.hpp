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
#include "Game/ActorHandle.hpp"

//-Forward-Declaration--------------------------------------------------------------------------------
class AIController;
class Controller;
class Map;
class PlayerController;
class Weapon;
struct ActorDefinition;
struct SpawnInfo;

// //----------------------------------------------------------------------------------------------------
// enum class eActorFaction : int8_t
// {
//     INVALID = -1,
//     MARINE,
//     DEMON,
//     NEUTRAL,
//     COUNT
// };

//----------------------------------------------------------------------------------------------------
class Actor
{
public:
    // Actor(Vec3 const& position, EulerAngles const& orientation, float radius, float height, bool isMovable, Rgba8 const& color);
    explicit Actor(SpawnInfo const& spawnInfo);

    void  Update();
    void  UpdatePosition();
    void  Render() const;
    Mat44 GetModelToWorldTransform() const;
    void  UpdatePhysics();
    void  Damage();
    void  AddForce();
    void  AddImpulse();
    void  OnCollide();
    void  OnPossessed(Controller* controller);
    void  OnUnpossessed();
    void  MoveInDirection();
    void  TurnInDirection();
    void  Attack();
    void  EquipWeapon();
    // SwitchInventory(unsigned int index);

    Vec3        m_position          = Vec3::ZERO;               // 3D position, as a Vec3, in world units.
    Vec3        m_velocity          = Vec3::ZERO;               // 3D velocity, as a Vec3, in world units per second.
    EulerAngles m_orientation       = EulerAngles::ZERO;        // 3D orientation, as EulerAngles, in degrees.
    EulerAngles m_angularVelocity   = EulerAngles::ZERO;
    float       m_radius            = 0.f;
    float       m_height            = 0.f;
    bool        m_isMovable         = false;
    Rgba8       m_color             = Rgba8::WHITE;
    Cylinder3   m_collisionCylinder = Cylinder3();

    ActorDefinition const* m_definition     = nullptr;      // A reference to our actor definition.
    bool                   m_isDead         = false;        // Any data needed to track if and how long we have been dead.
    int                    m_health         = 0;            // Current health.
    bool                   m_canBePossessed = false;
    float                  m_corpseLifetime = 0.f;
    bool                   m_isVisible      = false;
    Map*                   m_map            = nullptr;      // Reference to the map that spawned us.
    std::vector<Weapon*>   m_weapons;
    Controller*            m_controller = nullptr;          // A reference to the controller currently possessing us, if any.
    ActorHandle            m_handle;

    //----------------------------------------------------------------------------------------------------
    // A reference to our default AI controller, if any.
    // Used to keep track of our AI controller if the player possesses this actor,
    // in which case he pushes the AI out of the way until he releases possession.
    AIController* m_aiController = nullptr;    // AI controllers should be constructed by the actor when the actor is spawned and immediately possess that actor.
};
