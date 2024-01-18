#pragma once
#include <vector>
#include <unordered_map>
#include <typeinfo>
#include <typeindex>
#include <functional>
#include <cassert>
#include <algorithm>
#include <iostream>
#include <bitset>

// TODO: Need to create a better system for entity state, not sure i need each component to store a state, maybe just keep a map of entity to state in ECS.

typedef unsigned int Entity;

enum State {
    ACTIVE,
    SLEEP,
    DEAD,
};

struct Component {
    Entity owner;
};

template <typename T>
struct Components {
    std::vector<T> components;
    std::unordered_map<Entity, int> entity_to_index; // Mapping the entity id to the component index in components;
};

struct System {
    std::string name;           // For getting systems for manual calls
    unsigned int id;            // CURRENTLY UNUSED
    unsigned int requirements;  // Bit set of Component types

    std::vector<Entity> entities;

    std::function<void(struct AsteroidsGame& game, const System& system)> Action;
};

struct ECS {
    std::vector<Entity> entities;
    std::vector<void*> components;
    std::vector<System> systems; // NOTE: Pointer to systems as we inherit from System now but maybe just init the system type and set the Action instead!
    
    std::vector<unsigned int> on_update;

    std::unordered_map<Entity, unsigned int> entity_signature; // Capped at 32 signatures (Assuming 32 bit ints);
    std::unordered_map<Entity, State> entity_state; // Not a big fan of mapping this, will i ever really need all bits of a entity ID? Maybe use last bit to signal active/inactive? 28 bits might even be enough to store more data (~250m entities)
    std::unordered_map<std::type_index, unsigned int> component_to_id;
    std::unordered_map<Entity, unsigned int> entity_to_index; // Why is this here?

    void Update(AsteroidsGame& game) {
        for (int to_update: on_update) {
            systems[to_update].Action(game, systems[to_update]);
        }
    }

    template <typename T>
    const unsigned int Component() {
        return component_to_id[std::type_index(typeid(T))];
    }

       template <typename T>
    const unsigned int ComponentBits() {
        return (1 << component_to_id[std::type_index(typeid(T))]);
    }

    template <typename T>
    T& GetComponent(Entity entity) {
        const int components_index = component_to_id.at(std::type_index(typeid(T)));
        Components<T>& components_list = *(Components<T>*)components[components_index];
        const int entities_component_index = components_list.entity_to_index.at(entity);
        return components_list.components[entities_component_index];
    }

    Entity CreateEntity() {
        // This currently has no way to loop unused ids, if entities keep being created and destroyed for a long time we will eventually overflow!
        static unsigned int entity = 0;
        entities.push_back(entity);
        auto ent = entities[entities.size() - 1];
        entity_to_index[entity] = entities.size() - 1;
        entity++;

        return entities.back();
    }

    void RemoveEntity(Entity entity) {
        entities[entity_to_index[entity]] = entities.back();
        entities.pop_back();
    }

    template <typename T>
    void RegisterComponent() {
        component_to_id[std::type_index(typeid(T))] = components.size(); 

        Components<T>* component = new Components<T>(); // TODO: These are never freed.
        components.push_back((void*)component);
    }

    // Pass rather than template as we might want to call these systems our selves.
    void RegisterSystem(const System& system, bool call_every_update = true) {
        assert(system.Action);

        systems.push_back(system);

        if (call_every_update) on_update.push_back(systems.size() - 1);
    }

    System& GetSystem(std::string name) {
        for (System& system : systems) {
            if (system.name == name) return system;
        }
    }

    // TODO: Not a fan of this naming, Add...To Remove...From would be nice if only one placed changed for consistency.
    template <typename T>
    T& AddComponentToEntity(Entity entity) {
        int components_index = component_to_id[std::type_index(typeid(T))];
        Components<T>& components_list = *(Components<T>*)components[components_index];

        components_list.components.push_back(T{});
        components_list.components.back().owner = entity;
        components_list.entity_to_index[entity] = components_list.components.size() - 1;
        entity_signature[entity] |= ComponentBits<T>(); // Bitshift to get a unique bit for each component.

        for (System& system : systems) {
            if ((entity_signature[entity] & system.requirements) == system.requirements) {
                if (std::find(system.entities.begin(), system.entities.end(), entity) != system.entities.end()) continue;
                system.entities.push_back(entity);
            }
        }

        return components_list.components.back();
    }
    
    // TODO: As above
    template <typename T>
    void RemoveComponentFromEntity(Entity entity) {
        unsigned int old_signature = entity_signature[entity];
        entity_signature[entity] &= ~(1 << component_to_id[std::type_index(typeid(T))]); // And with the inverse of the component bit (keeps all other bits but this one);

        // Looping every system to check if this entity now is invalid for that system.
        for (System& system : systems) {
            if (
                ((old_signature & system.requirements) == system.requirements)              // Old system is valid
                &&                                                                          // BUT
                ((entity_signature[entity] & system.requirements) == system.requirements)   // New sytem isnt!
            ) {
                for (int i = 0; i < system.entities.size(); i++) {
                    if (system.entities[i] == entity) {
                        // Flip and pop
                        system.entities[i] = system.entities.back();
                        system.entities.pop_back();
                    }
                }
            }
        }
    }
};