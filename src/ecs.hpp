#include <vector>
#include <map>
#include <typeinfo>
#include <typeindex>

struct Entity {
    unsigned int id;
};

struct Component {
    Entity owner;
};

template <typename T>
struct Components {
    std::vector<T> components;
    std::map<Entity, int> entity_to_index; // Mapping the entity id to the component index in components;
};

struct System {
    unsigned int id;
    unsigned int requirements; // Bit set of Component types

    std::vector<Entity> entities;

    virtual void Action() = 0;
};

struct ECS {
    std::vector<Entity> entities;
    std::vector<void*> components;
    std::vector<System> systems;

    std::map<std::type_index, unsigned int> component_to_id;

    template <typename component_type>
    void RegisterComponents() {
        component_to_id[std::type_index(typeid(component_type))] = components.size(); 

        Components<component_type>* component = new Components<component_type>();
        components.push_back((void*)component);
    }

    template <typename T>
    void AddComponentToEntity(Entity entity) {
        components.push_back(T{});
        components[components.size() - 1].owner = entity;
        static_cast<Components<T>>(components[component_to_id[std::type_index(typeid(T))]]).entity_to_index[entity] = components.size() - 1;

        for (system: systems) {
            
        }
    }
};