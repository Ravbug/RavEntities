#pragma once
#include "Types.hpp"
#include "Registry.hpp"

struct World;

struct Entity{
    entity_t id = INVALID_ENTITY;
    
    Entity(entity_t id) : id(id){}
    Entity(){}
    
    template<typename T, typename ... A>
    inline T& EmplaceComponent(A ... args){
        return Registry::EmplaceComponent<T>(id, args...);
    }
    
    template<typename T>
    inline void DestroyComponent(){
        Registry::DestroyComponent<T>(id);
    }
    
    inline void Destroy(){
        Registry::DestroyEntity(id);
    }
    
    inline void MoveTo(World* newWorld){
        
    }
    
    // default create impl
    // define your own to hide this one
    inline void Create(){}
};
