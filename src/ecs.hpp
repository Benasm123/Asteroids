#include <vector>
#include <map>
#include <typeinfo>
#include <typeindex>

typedef unsigned int Entity;

struct Component {
    Entity owner;
};

template <typename T>
struct Components {
    std::vector<T> components;
    std::map<Entity, int> entity_to_index; // Mapping the entity id to the component index in components;
};

struct System {
    unsigned int id;            // CURRENTLY UNUSED
    unsigned int requirements;  // Bit set of Component types

    std::vector<Entity> entities;

    virtual void Action() = 0;
};

struct ECS {
    std::vector<Entity> entities;
    std::vector<void*> components;
    std::vector<System*> systems; // NOTE: Pointer to systems as we inherit from System now but maybe just init the system type and set the Action instead!
    
    std::vector<unsigned int> on_update;

    std::map<Entity, unsigned int> entity_signature; // Capped at 32 signatures (Assuming 32 bit ints);
    std::map<std::type_index, unsigned int> component_to_id;
    std::map<Entity, unsigned int> entity_to_index;

    void Update() {
        for (int to_update: on_update) {
            systems[to_update]->Action();
        }
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
    void RegisterComponents() {
        component_to_id[std::type_index(typeid(T))] = components.size(); 

        Components<T>* component = new Components<T>();
        components.push_back((void*)component);
    }

    // Pass rather than template as we might want to call these systems our selves.
    void RegisterSystem(System* system, bool call_every_update = true) {
        systems.push_back(system);

        if (call_every_update) on_update.push_back(systems.size() - 1);
    }

    // TODO: Not a fan of this naming, Add...To Remove...From would be nice if only one placed changed for consistency.
    template <typename T>
    void AddComponentToEntity(Entity entity) {
        components.push_back(T{});
        static_cast<Components<T>>(components[components.size() - 1]).owner = entity;
        static_cast<Components<T>>(components[component_to_id[std::type_index(typeid(T))]]).entity_to_index[entity] = components.size() - 1;
        entity_signature[entity] |= (1 << component_to_id[std::type_index(typeid(T))]); // Bitshift to get a unique bit for each component.

        for (System& system : systems) {
            if (entity_signature[entity] & system.requirements == system.requirements) {
                system.entities.push_back(entity);
            }
        }
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