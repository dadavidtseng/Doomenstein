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
    friend class Map;
    // friend class Game;
public:
    // Actor(Vec3 const& position, EulerAngles const& orientation, float radius, float height, bool isMovable, Rgba8 const& color);
    explicit Actor(SpawnInfo const& spawnInfo);

    void        Update(float deltaSeconds);
    void        UpdatePosition();
    void        Render() const;
    Mat44       GetModelToWorldTransform() const;
    Vec3        GetPosition() const;
    Vec3        GetEyePosition() const;
    float       GetCameraFOV() const;
    EulerAngles GetOrientation() const;
    void        UpdatePhysics(float deltaSeconds);
    void        Damage();
    void        AddForce(Vec3 const& force);
    void        AddImpulse(Vec3 const& impulse);
    void        MoveInDirection(Vec3 const& direction, float speed);
    void        TurnInDirection(EulerAngles & direction);
    void        OnCollide();
    void        OnPossessed(Controller* controller);
    void        OnUnpossessed();
    void        Attack();
    void        EquipWeapon();
    // SwitchInventory(unsigned int index);

    ActorHandle m_handle;
    // bool      m_isMovable         = false;
    bool m_isVisible = true;
    ActorDefinition const* m_definition     = nullptr;      // A reference to our actor definition.
    Vec3        m_position     = Vec3::ZERO;               // 3D position, as a Vec3, in world units.
    EulerAngles m_orientation  = EulerAngles::ZERO;        // 3D orientation, as EulerAngles, in degrees.


private:
    Vec3        m_velocity     = Vec3::ZERO;               // 3D velocity, as a Vec3, in world units per second.
    Vec3        m_acceleration = Vec3::ZERO;                // 3D acceleration, as a Vec3, in world units per second squared.
    // EulerAngles m_angularVelocity   = EulerAngles::ZERO;
    float     m_radius            = 0.f;
    float     m_height            = 0.f;
    float     m_eyeHeight         = 0.f;
    float m_cameraFOV            = 0.f;
    Rgba8     m_color             = Rgba8::WHITE;
    Cylinder3 m_collisionCylinder = Cylinder3();

    bool                   m_isDead         = false;        // Any data needed to track if and how long we have been dead.
    int                    m_health         = 0;            // Current health.
    bool                   m_canBePossessed = false;
    float                  m_corpseLifetime = 0.f;
    Map*                   m_map            = nullptr;      // Reference to the map that spawned us.
    std::vector<Weapon*>   m_weapons;
    Weapon*                m_currentWeapon = nullptr;
    Controller*            m_controller    = nullptr;          // A reference to the controller currently possessing us, if any.

    //----------------------------------------------------------------------------------------------------
    // A reference to our default AI controller, if any.
    // Used to keep track of our AI controller if the player possesses this actor,
    // in which case he pushes the AI out of the way until he releases possession.
    AIController* m_aiController = nullptr;    // AI controllers should be constructed by the actor when the actor is spawned and immediately possess that actor.
};
