#pragma once
#include "EntityRecordManager.hpp"
#include <functional>

template<typename ... A>
struct Entity{
    entity_id_t id;
    
    Entity() : id(EntityRecordManager::CreateEntity<A...>()){}
    
    inline void Destroy(){
        EntityRecordManager::DestroyEntity<A...>(id);
    }
    
    template<typename Comp_T, typename ... Args>
    inline void EmplaceComponent(Args ... ctor_args){
        // pass the current class Type
        EntityRecordManager::EmplaceComponent<decltype(std::remove_reference<decltype(*this)>()), Comp_T, A..., Args...>(id, ctor_args...);
    }
    
    template<typename Comp_T>
    inline void DestroyComponent(){
        EntityRecordManager::DestroyComponent<decltype(std::remove_reference<decltype(*this)>()),A...,Comp_T>(id);
    }
};

