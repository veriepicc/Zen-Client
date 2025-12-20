module;
#include <cstdint>
#include <string>
#include <memory>
#include <entt/entt.hpp>

export module Actor;

import Paul;
import Utils;
import MoveInputComponent;
import StateVectorComponent;
import ActorRotationComponent;
import RenderPositionComponent;
import AABBShapeComponent;
import ActorDataFlagComponent;
import ActorGameTypeComponent;
import OnGroundFlagComponent;
import RuntimeIDComponent;
import ActorEquipmentComponent;
import SynchedActorDataComponent;
import MobEffectsComponent;
import AttributesComponent;

class EntityId;

struct EntityIdTraits 
{
    using value_type   = EntityId;
    using entity_type  = std::uint32_t;
    using version_type = std::uint16_t;

    static constexpr entity_type entity_mask  = 0x3FFFF;
    static constexpr entity_type version_mask = 0x3FFF;
};

template <>
class entt::entt_traits<EntityId> : public entt::basic_entt_traits<EntityIdTraits> 
{
public:
    static constexpr entity_type page_size = 2048;
};

class EntityId : public entt::entt_traits<EntityId> 
{
public:
    entity_type rawId{};

    template <std::integral T>
    requires(!std::is_same_v<std::remove_cvref_t<T>, bool>)
    constexpr EntityId(T id) : rawId(static_cast<entity_type>(id)) {}

    constexpr bool isNull() const { return *this == entt::null; }
    constexpr operator entity_type() const { return rawId; }
    constexpr bool operator==(const EntityId& other) const { return rawId == other.rawId; }
};

class EntityRegistry : public std::enable_shared_from_this<EntityRegistry> 
{
public:
    std::string name;
    entt::basic_registry<EntityId> registry;
    std::uint32_t id;
};

struct EntityContext 
{
    EntityRegistry* registry;
    entt::basic_registry<EntityId>* enttRegistry;
    EntityId entityId;

    bool isValid() const 
    {
        return Utils::Mem::isValidPtr(registry) && Utils::Mem::isValidPtr(enttRegistry);
    }

    template <typename T>
    T* getComponent() 
    {
        return isValid() ? (T*)enttRegistry->try_get<T>(entityId) : nullptr;
    }
};

export class Actor 
{
public:
    EntityContext& getEntityContext() 
    {
        return *reinterpret_cast<EntityContext*>(reinterpret_cast<std::uintptr_t>(this) + 0x8);
    }

    // --- Component Accessors ---
    
    MoveInputComponent* getMoveInputComponent() { return getEntityContext().getComponent<MoveInputComponent>(); }
    StateVectorComponent* getStateVectorComponent() { return getEntityContext().getComponent<StateVectorComponent>(); }
    ActorRotationComponent* getActorRotationComponent() { return getEntityContext().getComponent<ActorRotationComponent>(); }
    RenderPositionComponent* getRenderPositionComponent() { return getEntityContext().getComponent<RenderPositionComponent>(); }
    AABBShapeComponent* getAABBShapeComponent() { return getEntityContext().getComponent<AABBShapeComponent>(); }
    ActorDataFlagComponent* getActorDataFlagComponent() { return getEntityContext().getComponent<ActorDataFlagComponent>(); }
    ActorGameTypeComponent* getActorGameTypeComponent() { return getEntityContext().getComponent<ActorGameTypeComponent>(); }
    OnGroundFlagComponent* getOnGroundFlagComponent() { return getEntityContext().getComponent<OnGroundFlagComponent>(); }
    RuntimeIDComponent* getRuntimeIDComponent() { return getEntityContext().getComponent<RuntimeIDComponent>(); }
    ActorEquipmentComponent* getActorEquipmentComponent() { return getEntityContext().getComponent<ActorEquipmentComponent>(); }
    SynchedActorDataComponent* getSynchedActorDataComponent() { return getEntityContext().getComponent<SynchedActorDataComponent>(); }
    MobEffectsComponent* getMobEffectsComponent() { return getEntityContext().getComponent<MobEffectsComponent>(); }
    AttributesComponent* getAttributesComponent() { return getEntityContext().getComponent<AttributesComponent>(); }

    // --- Helper Methods ---

    Paul::Vec3f* getPosition() 
    {
        auto* svc = getStateVectorComponent();
        return svc ? &svc->position : nullptr;
    }

    Paul::Vec3f* getVelocity() 
    {
        auto* svc = getStateVectorComponent();
        return svc ? &svc->velocity : nullptr;
    }

    Paul::Vec2f* getRotation() 
    {
        auto* arc = getActorRotationComponent();
        return arc ? &arc->rotation : nullptr;
    }

    bool isOnGround() { return getOnGroundFlagComponent() != nullptr; }
};
