//----------------------------------------------------------------------------------------------------
// Controller.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Game/ActorHandle.hpp"

class Camera;
//-Forward-Declaration--------------------------------------------------------------------------------
class Actor;
class Map;

//----------------------------------------------------------------------------------------------------
// Controllers may possess, and control, an actor.
class Controller
{
public:
    // Construction / Destruction
    explicit Controller(Map* owner);
    virtual  ~Controller() = default;

    virtual void   Update(float deltaSeconds) = 0;
    virtual void   Possess(ActorHandle const& actorHandle);
    virtual Actor* GetActor();

    // Setter
    void  SetControllerIndex(int index);
    AABB2 SetViewport(AABB2 const& viewPort);

    // Getter
    int GetControllerIndex() const;

    // The reference to controller's actor is by actor handle.
    ActorHandle m_actorHandle;           // Handle of our currently possessed actor or INVALID if no actor is possessed.
    Map*        m_map   = nullptr;    // Reference to the current map for purposes of dereferencing actor handles.
    int         m_index = -1;

    Camera* m_viewCamera = nullptr; // Handle screen message and hud
    AABB2   m_screenViewport;
    AABB2   m_viewport; // viewport size

    Camera*     m_worldCamera = nullptr; // Our camera. Used as the world camera when rendering.
    Vec3        m_position; // 3D position, separate from our actor so that we have a transform for the free-fly camera, as a Vec3, in world units.
    EulerAngles m_orientation; // 3D orientation, separate from our actor so that we have a transform for the free-fly camera, as EulerAngles, in degrees.
};
