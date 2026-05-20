#pragma once

#include <cstddef>
#include <utility>

#include <entt/entt.hpp>

namespace ant::core {

using Entity = entt::entity;

class EcsRegistry final {
public:
    Entity CreateEntity();
    void DestroyEntity(Entity entity);

    template <typename Component, typename... Args>
    Component& AddComponent(Entity entity, Args&&... args) {
        return registry_.emplace<Component>(entity, std::forward<Args>(args)...);
    }

    template <typename Component>
    Component& GetComponent(Entity entity) {
        return registry_.get<Component>(entity);
    }

    template <typename Component>
    const Component& GetComponent(Entity entity) const {
        return registry_.get<Component>(entity);
    }

    template <typename Component>
    bool HasComponent(Entity entity) const {
        return registry_.any_of<Component>(entity);
    }

    template <typename... Components>
    auto View() {
        return registry_.view<Components...>();
    }

    template <typename... Components>
    auto View() const {
        return registry_.view<Components...>();
    }

    std::size_t EntityCount() const;

    entt::registry& Raw();
    const entt::registry& Raw() const;

private:
    entt::registry registry_{};
};

} // namespace ant::core
