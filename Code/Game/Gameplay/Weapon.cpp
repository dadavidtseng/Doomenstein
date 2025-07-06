//----------------------------------------------------------------------------------------------------
// Weapon.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Gameplay/Weapon.hpp"

#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Gameplay/Actor.hpp"
#include "Game/Definition/ActorDefinition.hpp"
#include "Game/Framework/Animation.hpp"
#include "Game/Gameplay/Game.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Gameplay/HUD.hpp"
#include "Game/Gameplay/Map.hpp"
#include "Game/Definition/MapDefinition.hpp"
#include "Game/Framework/PlayerController.hpp"
#include "Game/Gameplay/Sound.hpp"
#include "Game/Definition/WeaponDefinition.hpp"

//----------------------------------------------------------------------------------------------------
Weapon::Weapon(Actor*            owner,
               WeaponDefinition* weaponDef)
    : m_owner(owner),
      m_definition(weaponDef)
{
    m_timer = new Timer(m_definition->m_refireTime, g_theGame->m_gameClock);
    m_timer->m_startTime = g_theGame->m_gameClock->GetTotalSeconds();
    m_animationTimer = new Timer(0.f, g_theGame->m_gameClock);

    /// Init hud base bound
    if (m_definition->m_hud != nullptr)
    {
        Texture const* baseTexture   = m_definition->m_hud->m_baseTexture;
        IntVec2 const  baseDimension = baseTexture->GetDimensions();
        float const    multiplier    = g_theGame->m_screenSpace.m_maxs.x / (float)baseDimension.x;
        m_hudBaseBound               = AABB2(Vec2(0.0f, 0.0f), Vec2(g_theGame->m_screenSpace.m_maxs.x, (float)baseDimension.y * multiplier));
    }
}

//----------------------------------------------------------------------------------------------------
Weapon::~Weapon()
{
    m_owner = nullptr;
}

void Weapon::Update(float const deltaSeconds)
{
    UpdateAnimation(deltaSeconds);
}

void Weapon::UpdateAnimation(float const deltaSeconds)
{
    UNUSED(deltaSeconds)
    if (!m_currentPlayingAnimation) return;

    if (m_animationTimer->GetElapsedTime() > m_currentPlayingAnimation->GetAnimationLength())
    {
        m_currentPlayingAnimation = nullptr;
        m_animationTimer->Stop();
    }
}

void Weapon::Render() const
{
    if (m_definition == nullptr || m_definition->m_hud == nullptr) return;

    g_theRenderer->BindShader(m_definition->m_hud->m_shader);
    g_theRenderer->SetBlendMode(eBlendMode::OPAQUE);
    RenderWeaponBase();
    RenderWeaponReticule();
    RenderWeaponAnim();
    RenderWeaponHudText();
}

void Weapon::RenderWeaponBase() const
{
    std::vector<Vertex_PCU> vertexes;
    vertexes.reserve(12000);

    //Vec2 spriteOffSet = -Vec2(m_definition->m_hud->m_spriteSize) * m_definition->m_hud->m_spritePivot;
    /// Hud Base TODO: Handle multiplayer
    Texture* baseTexture = m_definition->m_hud->m_baseTexture;
    AddVertsForAABB2D(vertexes, m_hudBaseBound, Rgba8::WHITE);
    g_theRenderer->BindTexture(baseTexture);
    g_theRenderer->DrawVertexArray(vertexes);
}

void Weapon::RenderWeaponReticule() const
{
    std::vector<Vertex_PCU> vertexes;
    vertexes.reserve(1024);
    /// Reticle
    Texture* reticleTexture   = m_definition->m_hud->m_reticuleTexture;
    Vec2     reticleDimension = Vec2(reticleTexture->GetDimensions().x, reticleTexture->GetDimensions().y);

    Vec2 reticleSpriteOffSet = -reticleDimension * m_definition->m_hud->m_spritePivot;

    AABB2 reticleBound = AABB2(Vec2(g_theGame->m_screenSpace.m_maxs / 2.0f), (Vec2(g_theGame->m_screenSpace.m_maxs / 2.0f) + Vec2(reticleDimension)));


    /// Change the dimension base on split screen y
    Vec2  dim            = reticleBound.GetDimensions();
    AABB2 screenViewport = m_owner->m_controller->m_screenViewport;
    float multiplier     = g_theGame->m_screenSpace.GetDimensions().y / screenViewport.GetDimensions().y;
    dim.x /= multiplier;
    reticleBound.SetDimensions(dim);
    /// End of Change

    AddVertsForAABB2D(vertexes, reticleBound, Rgba8::WHITE);
    g_theRenderer->BindTexture(reticleTexture);
    g_theRenderer->DrawVertexArray(vertexes);
}

void Weapon::RenderWeaponHudText() const
{
    g_theRenderer->BindTexture(nullptr);
    PlayerController* player = dynamic_cast<PlayerController*>(m_owner->m_controller);
    if (!player) return;
    std::vector<Vertex_PCU> vertexes;
    vertexes.reserve(1024);
    BitmapFont* g_testFont  = g_theRenderer->CreateOrGetBitmapFontFromFile("Data/Fonts/SquirrelFixedFont");
    AABB2       boundingBox = m_hudBaseBound;
    Vec2        dim         = boundingBox.GetDimensions();
    // m_owner->m_controller->m_screenViewport = boundingBox;
    AABB2 screenViewport = m_owner->m_controller->m_screenViewport;
    float multiplier     = g_theGame->m_screenSpace.GetDimensions().y / screenViewport.GetDimensions().y;
    g_testFont->AddVertsForTextInBox2D(vertexes, Stringf("%d", (int)m_owner->m_health), boundingBox, 40.f, Rgba8::WHITE, 1 / multiplier, Vec2(0.29f, 0.5f));
    // g_testFont->AddVertsForTextInBox2D(vertexes, Stringf("%d", PlayerSaveSubsystem::GetPlayerSaveData(player->m_index)->m_numOfKilled), boundingBox, 40.f, Rgba8::WHITE, 1 / multiplier,
    // Vec2(0.05f, 0.5f));
    // g_testFont->AddVertsForTextInBox2D(vertexes, Stringf("%d", PlayerSaveSubsystem::GetPlayerSaveData(player->m_index)->m_numOfDeaths), boundingBox, 40.f, Rgba8::WHITE, 1 / multiplier,
    // Vec2(0.95f, 0.5f));
    g_theRenderer->BindTexture(&g_testFont->GetTexture());
    g_theRenderer->DrawVertexArray(vertexes);
    // g_theRenderer->BindTexture(nullptr);
}

void Weapon::RenderWeaponAnim() const
{
    std::vector<Vertex_PCU> vertexes;
    vertexes.reserve(1024);
    Animation* animation = m_currentPlayingAnimation;
    if (animation == nullptr && (int)m_definition->m_hud->GetAnimations().size() > 0) // We use the index 0 animation group
    {
        animation = &m_definition->m_hud->GetAnimations()[0];
    }
    SpriteAnimDefinition const* anim         = animation->GetAnimationDefinition();
    SpriteDefinition const      spriteAtTime = anim->GetSpriteDefAtTime(m_animationTimer->GetElapsedTime());
    AABB2                       uvAtTime     = spriteAtTime.GetUVs();
    // DebuggerPrintf("(RenderWeaponAnim   %f\n", m_animationTimer->GetElapsedTime());
    Vec2 spriteOffSet = -Vec2(m_definition->m_hud->m_spriteSize) * m_definition->m_hud->m_spritePivot;

    IntVec2 boundSize = m_definition->m_hud->m_spriteSize;
    AABB2   bound     = AABB2(Vec2(g_theGame->m_screenSpace.m_maxs.x / 2.0f, 0.f), Vec2(g_theGame->m_screenSpace.m_maxs.x / 2.0f, 0.f) + Vec2(boundSize));

    /// Change the dimension base on split screen y
    // Vec2  dim            = bound.GetDimensions();
    // AABB2 screenViewport = m_owner->m_controller->m_screenViewport;
    // float multiplier     = g_theGame->m_screenSpace.GetDimensions().y / screenViewport.GetDimensions().y;
    // dim.x /= multiplier;
    // bound.SetDimensions(dim);
    /// End of Change

    bound.m_mins += spriteOffSet;
    bound.m_maxs += spriteOffSet;
    bound.Translate(Vec2(0.f, m_hudBaseBound.m_maxs.y)); // Shitty hardcode
    AddVertsForAABB2D(vertexes, bound, Rgba8::WHITE, uvAtTime.m_mins, uvAtTime.m_maxs);
    AddVertsForAABB2D(vertexes, uvAtTime, Rgba8::WHITE);
    g_theRenderer->BindTexture(&spriteAtTime.GetTexture());
    g_theRenderer->DrawVertexArray(vertexes);
}


//----------------------------------------------------------------------------------------------------
// Checks if the weapon is ready to fire.
// If so, fires each of the ray casts, projectiles, and melee attacks defined in the definition.
// Needs to pass along its owning actor to be ignored in all raycast and collision checks.
void Weapon::Fire()
{
    int rayCount        = m_definition->m_rayCount;
    int projectileCount = m_definition->m_projectileCount;
    int meleeCount      = m_definition->m_meleeCount;

    if (m_timer->HasPeriodElapsed())
    {
        float m_currentFireTime   = (float)g_theGame->m_gameClock->GetTotalSeconds();
        float m_timeSinceLastFire = m_currentFireTime - m_lastFireTime;
        if (m_timeSinceLastFire > m_definition->m_refireTime)
        {
            // m_owner->m_controller->m_state = "Attack";
            SoundID weaponFireSound = m_definition->GetSoundByName("Fire")->GetSoundID();
            g_theAudio->StartSoundAt(weaponFireSound, m_owner->m_position);
            if (m_definition->m_hud)
            {
                PlayAnimationByName("Attack");
            }

            PlayerController* player = dynamic_cast<PlayerController*>(m_owner->m_controller);
            if (player)
            {
                player->GetActor()->PlayAnimationByName("Attack");
            }

            m_timer->DecrementPeriodIfElapsed();
            m_lastFireTime = m_currentFireTime;
            // if (m_owner == nullptr) return;
            while (rayCount > 0)
            {
                //float             rayRange = m_definition->m_rayRange;
                Vec3              forward, left, up;
                Vec3 const        firePosition    = m_owner->m_position;
                Vec3 const        fireEyePosition = firePosition + Vec3(0.f, 0.f, m_owner->m_definition->m_eyeHeight);
                EulerAngles const fireOrientation = m_owner->m_orientation;
                fireOrientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
                ActorHandle           impactedActorHandle;
                RaycastResult3D const result        = m_owner->m_map->RaycastAll(m_owner, impactedActorHandle, fireEyePosition, forward, 10.f);
                Actor*                impactedActor = m_owner->m_map->GetActorByHandle(impactedActorHandle);

                if (result.m_didImpact)
                {
                    // DebugAddWorldPoint(result.m_impactPosition, 0.06f, 10.f);
                    // DebugAddWorldCylinder(fireEyePosition - Vec3::Z_BASIS * 0.05f, result.m_impactPosition, 0.01f, 10.f, false, Rgba8::BLUE, Rgba8::BLUE, DebugRenderMode::X_RAY);
                    // DebugAddWorldCylinder(fireEyePosition - Vec3::Z_BASIS * 0.05f, result.m_impactPosition, 0.01f, 10.f, false, Rgba8::BLUE, Rgba8::BLUE, DebugRenderMode::USE_DEPTH);
                    SpawnInfo particleSpawnInfo;
                    particleSpawnInfo.m_position   = result.m_impactPosition;
                    particleSpawnInfo.m_position.x = GetClamped(particleSpawnInfo.m_position.x, 0.f, 31.f);
                    particleSpawnInfo.m_position.y = GetClamped(particleSpawnInfo.m_position.y, 0.f, 31.f);
                    particleSpawnInfo.m_name       = "BulletHit";
                    g_theGame->m_currentMap->SpawnActor(particleSpawnInfo);
                }
                else
                {
                    // DebugAddWorldCylinder(fireEyePosition - Vec3::Z_BASIS * 0.05f, result.m_rayStartPosition + result.m_rayForwardNormal * rayRange, 0.01f, 10.f, false, Rgba8::BLUE, Rgba8::BLUE, DebugRenderMode::USE_DEPTH);
                }

                if (impactedActor != nullptr && impactedActor != m_owner)
                {
                    impactedActor->Damage((int)m_definition->m_rayDamage.m_min, m_owner->m_handle);
                    impactedActor->AddImpulse(m_definition->m_rayImpulse * forward);

                    //float damage = g_theRNG->RollRandomFloatInRange(m_definition->m_rayDamage.m_min, m_definition->m_rayDamage.m_max);

                    SpawnInfo particleSpawnInfo;
                    particleSpawnInfo.m_position = result.m_impactPosition;
                    particleSpawnInfo.m_name     = "BloodSplatter";
                    g_theGame->m_currentMap->SpawnActor(particleSpawnInfo);
                }
                rayCount--;
            }

            while (projectileCount > 0)
            {
                Vec3 startPos = m_owner->m_position + Vec3(0.f, 0.f, m_owner->m_definition->m_eyeHeight);
                Vec3 forward, left, up;

                EulerAngles randomDirection = GetRandomDirectionInCone(m_owner->m_orientation, m_definition->m_projectileCone);
                randomDirection.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
                Vec3      projectileSpeed = forward * m_definition->m_projectileSpeed;
                SpawnInfo spawnInfo;
                spawnInfo.m_name        = m_definition->m_projectileActor;
                spawnInfo.m_faction     = m_owner->m_definition->m_faction;
                spawnInfo.m_position    = startPos;
                spawnInfo.m_orientation = randomDirection;
                spawnInfo.m_velocity    = projectileSpeed;
                Actor* projectile       = m_owner->m_map->SpawnActor(spawnInfo);
                projectile->m_owner     = m_owner;
                projectileCount--;
            }

            while (meleeCount > 0)
            {
                meleeCount--;
                if (!m_owner || !m_owner->m_map) continue;

                // Forward vector from the owner's orientation
                Vec3 fwd, left, up;
                m_owner->m_orientation.GetAsVectors_IFwd_JLeft_KUp(fwd, left, up);

                Vec2  ownerPos2D = Vec2(m_owner->m_position.x, m_owner->m_position.y);
                Vec2  forward2D  = Vec2(fwd.x, fwd.y);
                float halfArc    = m_definition->m_meleeArc * 0.5f;

                Actor* bestTarget   = nullptr;
                float  bestDistSq   = FLT_MAX;
                float  meleeRangeSq = m_definition->m_meleeRange * m_definition->m_meleeRange;

                for (Actor* testActor : m_owner->m_map->m_actors)
                {
                    if (!testActor || testActor == m_owner) continue;
                    if (testActor->m_isDead) continue;
                    if (testActor->m_definition->m_faction == m_owner->m_definition->m_faction) continue;
                    if (testActor->m_definition->m_faction == "NEUTRAL" || m_owner->m_definition->m_faction == "NEUTRAL") continue;

                    Vec2  testPos2D   = Vec2(testActor->m_position.x, testActor->m_position.y);
                    float distSquared = GetDistanceSquared2D(ownerPos2D, testPos2D);
                    if (distSquared > meleeRangeSq) continue;
                    Vec2  toTarget2D = (testPos2D - ownerPos2D).GetNormalized();
                    float angle      = GetAngleDegreesBetweenVectors2D(forward2D, toTarget2D);
                    if (angle > halfArc) continue;

                    if (distSquared < bestDistSq)
                    {
                        bestDistSq = distSquared;
                        bestTarget = testActor;
                    }
                }
                if (bestTarget)
                {
                    float damage = g_theRNG->RollRandomFloatInRange(m_definition->m_meleeDamage.m_min, m_definition->m_meleeDamage.m_max);
                    bestTarget->Damage((int)damage, m_owner->m_handle);
                    bestTarget->AddImpulse(m_definition->m_meleeImpulse * fwd);
                }
            }
        }
    }
}

Animation* Weapon::PlayAnimationByName(std::string animationName, bool force)
{
    Animation* weaponAnim = m_definition->m_hud->GetAnimationByName(animationName);
    if (weaponAnim)
    {
        if (weaponAnim == m_currentPlayingAnimation)
        {
            DebuggerPrintf("CHECK C %f\n", m_animationTimer->GetElapsedTime());

            return weaponAnim;
        }

        /// We want to replace to new animation, force update it whether or not it finished
        if (force)
        {
            m_currentPlayingAnimation = weaponAnim;
            m_animationTimer->Start();
            return weaponAnim;
        }

        if (m_currentPlayingAnimation)
        {
            DebuggerPrintf("CHECK A %f\n", m_animationTimer->GetElapsedTime());

            bool isCurrentAnimFinished = m_animationTimer->GetElapsedTime() >= m_currentPlayingAnimation->GetAnimationLength();
            if (isCurrentAnimFinished)
            {
                m_currentPlayingAnimation = weaponAnim;
                m_animationTimer->Start();
                return weaponAnim;
            }
        }
        else
        {
            DebuggerPrintf("CHECK B %f\n", m_animationTimer->GetElapsedTime());

            m_currentPlayingAnimation = weaponAnim;
            m_animationTimer->Start();
            return weaponAnim;
        }
    }
    return nullptr;
}

//----------------------------------------------------------------------------------------------------
// This, and other utility methods, will be helpful for randomizing weapons with a cone.
EulerAngles Weapon::GetRandomDirectionInCone(EulerAngles weaponOrientation, float degreeOfVariation)
{
    float const       randomYaw       = g_theRNG->RollRandomFloatInRange(-degreeOfVariation, degreeOfVariation);
    float const       randomPitch     = g_theRNG->RollRandomFloatInRange(-degreeOfVariation, degreeOfVariation);
    float const       randomRow       = g_theRNG->RollRandomFloatInRange(-degreeOfVariation, degreeOfVariation);
    EulerAngles const randomDirection = EulerAngles(weaponOrientation.m_yawDegrees + randomYaw, weaponOrientation.m_pitchDegrees + randomPitch, weaponOrientation.m_rollDegrees + randomRow);
    return randomDirection;
}
