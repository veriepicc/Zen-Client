module;
#include <vector>
#include <entt/entt.hpp>
#include <cstdint>

export module Global;

import ClientInstance;
import LocalPlayer;
import Actor;
import ActorOwnerComponent;
import Utils;
import GLMatrix;
import Paul;

export namespace Global 
{
    inline ClientInstance* clientInstance = nullptr;

    void setClientInstance(ClientInstance* instance) { clientInstance = instance; }
    ClientInstance* getClientInstance() { return clientInstance; }

    inline GLMatrix viewMatrix;
    inline Paul::Vec3f cameraPos;
    inline Paul::Vec2f fov;

    LocalPlayer* getLocalPlayer() 
    {
        return clientInstance ? clientInstance->getLocalPlayer() : nullptr;
    }

    std::vector<Actor*> getActorList() 
    {
        std::vector<Actor*> actors;
        auto* player = getLocalPlayer();
        if (!player) return actors;

        auto& context = player->getEntityContext();
        if (!context.isValid()) return actors;

        auto view = context.enttRegistry->view<ActorOwnerComponent>();
        view.each([&](auto entity, auto& owner) {
            if (Utils::Mem::isModulePtr(owner.actor)) {
                actors.push_back(owner.actor);
            }
        });

        return actors;
    }
}