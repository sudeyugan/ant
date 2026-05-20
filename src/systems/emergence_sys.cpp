#include "emergence_sys.h"

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

#include <entt/entt.hpp>

#include "core/ecs_registry.h"
#include "core/event_bus.h"
#include "core/events.h"
#include "components/agent.h"
#include "components/social.h"
#include "components/transform.h"
#include "spatial/grid_hash.h"

namespace ant::systems {

void EmergenceSystem::Update(ant::core::EcsRegistry& registry, const ant::spatial::GridHash& grid_hash) {
    auto& reg = registry.Raw();
    auto state_view = reg.view<ant::components::EmergenceState>();
    if (state_view.begin() == state_view.end()) {
        return;
    }

    auto state_entity = *state_view.begin();
    auto& state = reg.get<ant::components::EmergenceState>(state_entity);

    CheckFamilyEmergence(registry, grid_hash, state);
    CheckOrganizationEmergence(registry, grid_hash, state);
    CheckLeadershipTransfer(registry);
}

void EmergenceSystem::CheckFamilyEmergence(ant::core::EcsRegistry& registry,
                                           const ant::spatial::GridHash& grid_hash,
                                           ant::components::EmergenceState& state) {
    auto& reg = registry.Raw();
    auto view = reg.view<ant::components::Position, ant::components::Wealth>();

    // Find unaffiliated pairs in proximity
    for (auto it = view.begin(); it != view.end(); ++it) {
        entt::entity e1 = *it;
        if (reg.any_of<ant::components::FamilyComponent>(e1)) continue;

        auto& pos1 = view.get<ant::components::Position>(e1);
        
        // Search for nearby unaffiliated entities
        std::vector<entt::entity> nearby;
        grid_hash.QueryRadius(pos1, 1.5f, nearby);
        
        for (entt::entity e2 : nearby) {
            if (e1 == e2 || !reg.valid(e2)) continue;
            if (reg.any_of<ant::components::FamilyComponent>(e2)) continue;
            if (!reg.any_of<ant::components::Position>(e2)) continue;

            auto& pos2 = reg.get<ant::components::Position>(e2);
            
            // Check distance
            float dx = pos1.x - pos2.x;
            float dy = pos1.y - pos2.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            
            if (dist < 1.0f) {
                // Potential family formation: check if wealth transfer happens
                auto& wealth1 = reg.get<ant::components::Wealth>(e1);
                auto& wealth2 = reg.get<ant::components::Wealth>(e2);
                
                // Simple heuristic: if both have decent resources, form family
                if (wealth1.coins > 0.5f && wealth2.coins > 0.5f) {
                    uint64_t family_id = state.next_family_id++;
                    
                    reg.emplace_or_replace<ant::components::FamilyComponent>(e1, family_id, true);
                    reg.emplace_or_replace<ant::components::FamilyComponent>(e2, family_id, false);
                    
                    ant::core::EventBus::Get().Publish<ant::core::EventFamilyFormed>(family_id, e1, e2);
                    break;  // Only one family per entity per tick
                }
            }
        }
    }
}

void EmergenceSystem::CheckOrganizationEmergence(ant::core::EcsRegistry& registry,
                                                 const ant::spatial::GridHash& grid_hash,
                                                 ant::components::EmergenceState& state) {
    auto& reg = registry.Raw();
    auto view = reg.view<ant::components::Position, ant::components::Wealth>();

    // Sample a few positions to detect dense clusters
    std::vector<entt::entity> entities;
    for (auto e : view) {
        if (!reg.any_of<ant::components::MemberComponent>(e)) {
            entities.push_back(e);
        }
    }

    // Every 100 ticks, try to form organization in a dense zone
    state.org_check_counter++;
    
    if (state.org_check_counter < 100 || entities.size() < 10) return;
    state.org_check_counter = 0;

    // Pick random entity and check cluster size in radius 8.0
    if (entities.empty()) return;
    
    static thread_local std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<std::size_t> pick(0, entities.size() - 1);
    entt::entity leader = entities[pick(rng)];
    auto& leader_pos = reg.get<ant::components::Position>(leader);

    std::vector<entt::entity> cluster;
    grid_hash.QueryRadius(leader_pos, 8.0f, cluster);
    
    // Filter unaffiliated members
    std::vector<entt::entity> unaffiliated;
    float max_wealth = reg.get<ant::components::Wealth>(leader).coins;
    entt::entity wealth_leader = leader;
    
    for (entt::entity e : cluster) {
        if (!reg.valid(e) || !reg.any_of<ant::components::Wealth>(e)) continue;
        if (reg.any_of<ant::components::MemberComponent>(e)) continue;
        
        unaffiliated.push_back(e);
        
        float wealth = reg.get<ant::components::Wealth>(e).coins;
        if (wealth > max_wealth) {
            max_wealth = wealth;
            wealth_leader = e;
        }
    }

    // Emergence threshold: 5+ unaffiliated members
    if (unaffiliated.size() >= 5) {
        uint64_t org_id = state.next_org_id++;
        
        // Create virtual entity to represent organization
        entt::entity org_entity = reg.create();
        reg.emplace<ant::components::Position>(org_entity, reg.get<ant::components::Position>(wealth_leader));
        
        std::vector<entt::entity> members;
        float total_wealth = 0.0f;
        
        for (entt::entity member : unaffiliated) {
            reg.emplace<ant::components::MemberComponent>(member, org_id);
            members.push_back(member);
            total_wealth += reg.get<ant::components::Wealth>(member).coins;
        }

        reg.emplace<ant::components::OrganizationComponent>(
            org_entity, org_id, wealth_leader, members, total_wealth
        );

        ant::core::EventBus::Get().Publish<ant::core::EventOrganizationFormed>(
            org_id, wealth_leader, static_cast<uint32_t>(members.size())
        );
    }
}

void EmergenceSystem::CheckLeadershipTransfer(ant::core::EcsRegistry& registry) {
    auto& reg = registry.Raw();
    auto view = reg.view<ant::components::OrganizationComponent>();
    std::vector<entt::entity> orgs_to_destroy;

    for (auto org_entity : view) {
        auto& org = reg.get<ant::components::OrganizationComponent>(org_entity);
        
        // Check if leader is still alive
        if (!reg.valid(org.leader) || !reg.any_of<ant::components::Wealth>(org.leader)) {
            // Leader died; pick richest valid member as new leader
            bool found_leader = false;
            entt::entity new_leader = entt::null;
            float max_wealth = 0.0f;

            for (entt::entity member : org.members) {
                if (!reg.valid(member) || !reg.any_of<ant::components::Wealth>(member)) {
                    continue;
                }
                float wealth = reg.get<ant::components::Wealth>(member).coins;
                if (!found_leader || wealth > max_wealth) {
                    found_leader = true;
                    max_wealth = wealth;
                    new_leader = member;
                }
            }

            if (!found_leader) {
                for (entt::entity member : org.members) {
                    if (reg.valid(member) && reg.any_of<ant::components::MemberComponent>(member)) {
                        reg.remove<ant::components::MemberComponent>(member);
                    }
                }
                orgs_to_destroy.push_back(org_entity);
                ant::core::EventBus::Get().Publish<ant::core::EventOrganizationDissolved>(org.org_id);
                continue;
            }

            entt::entity old_leader = org.leader;
            org.leader = new_leader;
            
            ant::core::EventBus::Get().Publish<ant::core::EventLeadershipTransfer>(
                org.org_id, old_leader, new_leader
            );
        } else {
            // Recalculate total wealth
            float total = 0.0f;
            for (entt::entity member : org.members) {
                if (reg.valid(member) && reg.any_of<ant::components::Wealth>(member)) {
                    total += reg.get<ant::components::Wealth>(member).coins;
                }
            }
            org.total_wealth = total;
        }
    }

    for (entt::entity org_entity : orgs_to_destroy) {
        if (reg.valid(org_entity)) {
            reg.destroy(org_entity);
        }
    }
}

}  // namespace ant::systems
