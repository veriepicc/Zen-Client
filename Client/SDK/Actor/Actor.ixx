module;
#include <cstdint>
#include <string>
#include <memory>
#include <entt/entt.hpp>

export module Actor;

import Paul;
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


// Minecraft uses a custom entity ID with specific bit layout for versioning
class EntityId;

struct EntityIdTraits
{
    using value_type   = EntityId;
    using entity_type  = std::uint32_t;
    using version_type = std::uint16_t;

    static constexpr entity_type entity_mask  = 0x3FFFF;
    static constexpr entity_type version_mask = 0x3FFF;
};

template<>
class entt::entt_traits<EntityId> : public entt::basic_entt_traits<EntityIdTraits>
{
public:
    static constexpr entity_type page_size = 2048;
};

class EntityId : public entt::entt_traits<EntityId>
{
public:
    entity_type rawId{};

    template<std::integral IntegralType>
    requires(!std::is_same_v<std::remove_cvref_t<IntegralType>, bool>)
    constexpr EntityId(IntegralType id) : rawId(static_cast<entity_type>(id)) {}

    constexpr bool isNull() const { return *this == entt::null; }
    constexpr operator entity_type() const { return rawId; }
    constexpr bool operator==(const EntityId& other) const { return rawId == other.rawId; }
};


// The game's entity registry wrapper
class EntityRegistry : public std::enable_shared_from_this<EntityRegistry>
{
public:
    std::string name;
    entt::basic_registry<EntityId> registry;
    std::uint32_t id;
};


// The context that binds an entity to its registry
struct EntityContext
{
    EntityRegistry& owningRegistry;
    entt::basic_registry<EntityId>& componentRegistry;
    EntityId entityId;

    bool isValid() const
    {
        return componentRegistry.valid(entityId);
    }

    template<typename ComponentType>
    ComponentType* getComponent()
    {
        if (!isValid()) return nullptr;
        
        // For components that might be empty/tags, we cast the result of try_get 
        // to handle cases where entt returns void* or optimized types.
        return (ComponentType*)componentRegistry.try_get<ComponentType>(entityId);
    }
};


export class Actor
{
public:
    EntityContext& getEntityContext()
    {
        // EntityContext lives at offset 0x8 from Actor base
        return *reinterpret_cast<EntityContext*>(reinterpret_cast<std::uintptr_t>(this) + 0x8);
    }

    MoveInputComponent* getMoveInputComponent()
    {
        auto& context = getEntityContext();
        return context.getComponent<MoveInputComponent>();
    }

    StateVectorComponent* getStateVectorComponent()
    {
        auto& context = getEntityContext();
        return context.getComponent<StateVectorComponent>();
    }
    
    ActorRotationComponent* getActorRotationComponent()
    {
        auto& context = getEntityContext();
        return context.getComponent<ActorRotationComponent>();
    }
    
    RenderPositionComponent* getRenderPositionComponent()
    {
        auto& context = getEntityContext();
        return context.getComponent<RenderPositionComponent>();
    }
    
    AABBShapeComponent* getAABBShapeComponent()
    {
        auto& context = getEntityContext();
        return context.getComponent<AABBShapeComponent>();
    }
    
    ActorDataFlagComponent* getActorDataFlagComponent()
    {
        auto& context = getEntityContext();
        return context.getComponent<ActorDataFlagComponent>();
    }
    
    ActorGameTypeComponent* getActorGameTypeComponent()
    {
        auto& context = getEntityContext();
        return context.getComponent<ActorGameTypeComponent>();
    }
    
    OnGroundFlagComponent* getOnGroundFlagComponent()
    {
        auto& context = getEntityContext();
        return context.getComponent<OnGroundFlagComponent>();
    }
    
    RuntimeIDComponent* getRuntimeIDComponent()
    {
        auto& context = getEntityContext();
        return context.getComponent<RuntimeIDComponent>();
    }
    
    ActorEquipmentComponent* getActorEquipmentComponent()
    {
        auto& context = getEntityContext();
        return context.getComponent<ActorEquipmentComponent>();
    }
    
    SynchedActorDataComponent* getSynchedActorDataComponent()
    {
        auto& context = getEntityContext();
        return context.getComponent<SynchedActorDataComponent>();
    }
    
    MobEffectsComponent* getMobEffectsComponent()
    {
        auto& context = getEntityContext();
        return context.getComponent<MobEffectsComponent>();
    }
    
    AttributesComponent* getAttributesComponent()
    {
        auto& context = getEntityContext();
        return context.getComponent<AttributesComponent>();
    }

    Paul::Vec3f* getPosition()
    {
        auto* stateVector = getStateVectorComponent();
        if (!stateVector) return nullptr;
        return &stateVector->position;
    }

    Paul::Vec3f* getVelocity()
    {
        auto* stateVector = getStateVectorComponent();
        if (!stateVector) return nullptr;
        return &stateVector->velocity;
    }
    
    Paul::Vec2f* getRotation()
    {
        auto* rotationComponent = getActorRotationComponent();
        if (!rotationComponent) return nullptr;
        return &rotationComponent->rotation;
    }
    
    bool isOnGround()
    {
        return getOnGroundFlagComponent() != nullptr;
    }
};
