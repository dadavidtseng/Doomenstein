//----------------------------------------------------------------------------------------------------
// Actor.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include <map>
#include <vector>

//----------------------------------------------------------------------------------------------------
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/Cylinder3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Game/Framework/ActorHandle.hpp"
#include "Game/Gameplay/Sound.hpp"

//-Forward-Declaration--------------------------------------------------------------------------------
class AIController;
class AnimationGroup;
class Controller;
class PlayerController;
class Texture;
class Timer;
class Weapon;
struct AABB3;
struct ActorDefinition;
struct SpawnInfo;

//----------------------------------------------------------------------------------------------------
class Actor
{
    friend class Map;

public:
    explicit Actor(SpawnInfo const& spawnInfo);

    void  Update(float deltaSeconds);
    void  Render(PlayerController const* toPlayer) const;
    Mat44 GetModelToWorldTransform() const;

    void UpdatePhysics(float deltaSeconds);
    void UpdateAnimation(float deltaSeconds);
    void Damage(int damage, ActorHandle const& other);
    void AddForce(Vec3 const& force);
    void AddImpulse(Vec3 const& impulse);
    void MoveInDirection(Vec3 const& direction, float speed);
    void TurnInDirection(EulerAngles const& direction);

    // Possession
    void OnPossessed(Controller* controller);
    void OnUnpossessed();

    // Collision
    void OnCollisionEnterWithActor(Actor* other);
    void OnCollisionEnterWithMap(IntVec2 const& tileCoords);
    void OnCollisionEnterWithMap(AABB3 const& bounds);

    void Attack() const;
    void SwitchInventory(unsigned int index);
    Vec3 GetActorEyePosition() const;

    AnimationGroup* PlayAnimationByName(String const& animationName, bool force = false);


    ActorHandle      m_handle;
    ActorDefinition* m_definition = nullptr;      // A reference to our actor definition.
    Actor*           m_owner      = nullptr;

    // bool        m_isVisible    = true;
    bool        m_isStatic     = false;
    Vec3        m_position     = Vec3::ZERO;               // 3D position, as a Vec3, in world units.
    Vec3        m_velocity     = Vec3::ZERO;               // 3D velocity, as a Vec3, in world units per second.
    Vec3        m_acceleration = Vec3::ZERO;                // 3D acceleration, as a Vec3, in world units per second squared.
    EulerAngles m_orientation  = EulerAngles::ZERO;        // 3D orientation, as EulerAngles, in degrees.

    float                m_radius            = 0.f;
    float                m_height            = 0.f;
    Rgba8                m_color             = Rgba8::WHITE;
    Cylinder3            m_collisionCylinder = Cylinder3();
    String               m_weaponName;
    float                m_dead           = 0.f;
    bool                 m_isDead         = false;        // Any data needed to track if and how long we have been dead.
    bool                 m_isGarbage      = false;
    int                  m_health         = 0;            // Current health.
    bool                 m_canBePossessed = false;
    float                m_corpseLifetime = 0.f;
    Map*                 m_map            = nullptr;      // Reference to the map that spawned us.
    std::vector<Weapon*> m_weapons;
    Weapon*              m_currentWeapon = nullptr;
    Controller*          m_controller    = nullptr;          // A reference to the controller currently possessing us, if any.
    Texture*             m_texture       = nullptr;

    AnimationGroup* m_currentPlayingAnimationGroup  = nullptr;
    Timer*          m_animationTimer                = nullptr;
    float           m_animationTimerSpeedMultiplier = 1.f;

    //----------------------------------------------------------------------------------------------------
    // A reference to our default AI controller, if any.
    // Used to keep track of our AI controller if the player possesses this actor,
    // in which case he pushes the AI out of the way until he releases possession.
    AIController*                      m_aiController = nullptr;    // AI controllers should be constructed by the actor when the actor is spawned and immediately possess that actor.
    std::map<SoundID, SoundPlaybackID> m_soundPlaybackIDs;
};
