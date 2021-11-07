#pragma once
#include "Types.hpp"
#include <queue>
#include <vector>
#include "World.hpp"
#include <cassert>

struct World;

class Registry{
    
    friend class World;
    friend class Entity;
    
    struct EntityData{
        World* world = nullptr;
        entity_t idInWorld = INVALID_ENTITY;
        EntityData(decltype(world) w, decltype(idInWorld) i) : world(w), idInWorld(i){}
    };
    
    static std::queue<entity_t> available;
    static std::vector<EntityData> entityData;
    
    // invoked by the world
    static inline entity_t CreateEntity(World* world, const entity_t idInWorld){
        entity_t id;
        if (available.size() > 0){
            id = available.front();
            available.pop();
            auto& data = entityData[id];
            data.idInWorld = idInWorld;
            data.world = world;
        }
        else{
            id = entityData.size();
            entityData.emplace_back(world,idInWorld);
        }
        return id;
    }
    
    // invoked by the world
    static inline void DestroyEntity(entity_t global_id){
        // make this entity's ID available for reuse
        available.push(global_id);
        
        auto& data = entityData[global_id];
        data.world->Destroy(data.idInWorld);        
    }
    
    template<typename T, typename ... A>
    static inline T& EmplaceComponent(entity_t id, A ... args){
        // get the world
        assert(EntityIsValid(id));
        auto& data = entityData[id];
        return data.world->EmplaceComponent<T>(data.idInWorld,args...);
    }
    
    template<typename T>
    static inline void DestroyComponent(entity_t id){
        assert(EntityIsValid(id));
        auto& data = entityData[id];
        data.world->DestroyComponent<T>(data.idInWorld);
    }
    
    template<typename T>
    static inline T& GetComponent(entity_t id) {
        assert(EntityIsValid(id));
        auto& data = entityData[id];
        return data.world->GetComponent<T>(data.idInWorld);
    }

    template<typename T>
    static inline bool HasComponent(entity_t id) {
        assert(EntityIsValid(id));
        auto& data = entityData[id];
        return data.world->HasComponent<T>(data.idInWorld);
    }
};
